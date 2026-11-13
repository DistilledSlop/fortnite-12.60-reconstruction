#!/usr/bin/env python3
"""Read-only 12.60 client equipment snapshot. Supply PID or PID:current-pawn for each client."""
import argparse
import math
import struct
import json
import os
from pathlib import Path
import tempfile
import time

# Original 12.60 executable image addresses and UObject inheritance-table layout,
# from the paired-client actor probe and pc_probe.py. No memory-map scanning.
RLO, RHI = 0x1458fc000, 0x147811c00
PAWN_CLASS = 0x1481b5e50
DRIVER, SLOT = 0x1423bc750, 0x1570


def read(mem, address, fmt):
    try:
        size = struct.calcsize(fmt)
        data = os.pread(mem, size, address)
        return struct.unpack(fmt, data) if len(data) == size else None
    except (OSError, OverflowError, ValueError, struct.error):
        return None


def one(mem, address, fmt):
    result = read(mem, address, fmt)
    return result[0] if result else None


def clean(value):
    if isinstance(value, tuple):
        return [clean(item) for item in value]
    return None if isinstance(value, float) and not math.isfinite(value) else value


def pawn_actor(mem, address, level, base):
    if one(mem, address + 0x20, '<Q') != level:
        return False
    cls = one(mem, address + 0x10, '<Q')
    if not cls or not base:
        return False
    depth = one(mem, cls + 0x38, '<i')
    base_depth = one(mem, base + 0x38, '<i')
    table = one(mem, cls + 0x30, '<Q')
    return (depth is not None and base_depth is not None and
            0 <= base_depth <= depth < 256 and bool(table) and
            one(mem, table + base_depth * 8, '<Q') == base + 0x30)


def req(mem, addr, fmt, label):
    value = read(mem, addr, fmt)
    if value is None:
        raise RuntimeError(f'unreadable {label} at {addr:#x}')
    return value[0] if len(value) == 1 else clean(value)


def ptr(value):
    return hex(value) if value else None


def obj(mem, address):
    if not address:
        return None
    vt = req(mem, address, '<Q', 'object vtable')
    cls = req(mem, address + 0x10, '<Q', 'object class')
    if not (RLO <= vt < RHI and cls and one(mem, cls, '<Q')):
        raise RuntimeError(f'object {address:#x} has invalid vtable/class')
    return {'address': ptr(address), 'class': ptr(cls),
            'name_fname_raw': req(mem, address + 0x18, '<II', 'object name'),
            'class_fname_raw': req(mem, cls + 0x18, '<II', 'class name')}


def component(mem, address):
    if not address:
        return None
    return {'object': obj(mem, address),
            'attach_parent': ptr(req(mem, address + 0xc0, '<Q', 'attach parent')),
            'attach_socket_fname_raw': req(mem, address + 0xc8, '<II', 'attach socket'),
            'relative_location': req(mem, address + 0x11c, '<3f', 'relative location'),
            'relative_rotation': req(mem, address + 0x128, '<3f', 'relative rotation'),
            'relative_scale': req(mem, address + 0x134, '<3f', 'relative scale'),
            'component_world_quaternion_xyzw': req(mem, address + 0x1c0, '<4f', 'world quaternion'),
            'component_world_translation': req(mem, address + 0x1d0, '<3f', 'world translation')}


def level_actors(mem, level):
    addresses, counts = set(), []
    for off in (0x98, 0xa8):
        arr = read(mem, level + off, '<Qii')
        if not arr or not (0 <= arr[1] <= arr[2] <= 200000 and (arr[0] or arr[1] == 0)):
            raise RuntimeError(f'invalid ULevel array at {level + off:#x}')
        entries = read(mem, arr[0], '<' + str(arr[1]) + 'Q') if arr[1] else ()
        if entries is None:
            raise RuntimeError(f'unreadable ULevel array at {level + off:#x}')
        counts.append(arr[1]); addresses.update(entries)
    return addresses, counts


FLAGS = {'crouched': (0x320, 1), 'dbno': (0x53a, 2),
         'skydiving': (0x10e5, 1), 'parachute_open': (0x10e6, 1),
         'local_skydiving': (0x10e7, 1), 'parachute_forced_open': (0x10e7, 2),
         'skydiving_from_bus': (0x10e7, 4), 'skydiving_from_launchpad': (0x10e7, 8),
         'pending_skydive_launch': (0x10e7, 16), 'targeting': (0x1062, 16),
         'sprint_jump': (0x1061, 16), 'parachute_locked': (0x2284, 1),
         'local_parachute_locked': (0x2285, 1), 'weapon_holstered': (0x539, 16)}


def discover_context(pid):
    """Follow the original engine/viewport/world chain; never scan process mappings.

    GEngine 0x148351eb8 is read by original GetPlayerController 0x144215e10.
    GameViewport (+0x7c0) and viewport World (+0x78) are reflected properties;
    World PersistentLevel (+0x30) is checked by the phase probe as well.
    """
    mem = os.open(f'/proc/{pid}/mem', os.O_RDONLY)
    try:
        engine = req(mem, 0x148351eb8, '<Q', 'GEngine')
        if not engine:
            raise RuntimeError('GEngine is not initialized')
        obj(mem, engine)
        viewport = req(mem, engine + 0x7c0, '<Q', 'game viewport')
        if not viewport:
            raise RuntimeError('game viewport is not initialized')
        obj(mem, viewport)
        world = req(mem, viewport + 0x78, '<Q', 'viewport world')
        if not world:
            raise RuntimeError('viewport world is not initialized')
        obj(mem, world)
        level = req(mem, world + 0x30, '<Q', 'persistent level')
        if not level:
            raise RuntimeError('persistent level is unavailable')
        obj(mem, level)
        actors, _ = level_actors(mem, level)
        candidates = []
        for actor in actors:
            vt = one(mem, actor, '<Q') if actor else None
            if (vt and RLO <= vt < RHI and one(mem, vt + SLOT, '<Q') == DRIVER
                    and one(mem, actor + 0xf0, '<B') == 2
                    and one(mem, actor + 0x20, '<Q') == level
                    and one(mem, actor + 0x220, '<Q')):
                obj(mem, actor)
                pawn = one(mem, actor + 0x248, '<Q')
                if pawn and (pawn not in actors or one(mem, pawn + 0x250, '<Q') != actor):
                    continue
                candidates.append({'pc': actor, 'pawn': pawn, 'world': world, 'level': level})
        if len(candidates) != 1:
            raise RuntimeError(f'expected one local controller, got {len(candidates)}')
        return candidates[0]
    finally:
        os.close(mem)


def discover_pawn(pid):
    context = discover_context(pid)
    if not context['pawn']:
        raise RuntimeError('local controller has no possessed pawn')
    return context['pawn']


def snapshot(pid, supplied):
    if supplied is None:
        supplied = discover_pawn(pid)
    started = time.time_ns()
    mem = os.open(f'/proc/{pid}/mem', os.O_RDONLY)
    try:
        level = req(mem, supplied + 0x20, '<Q', 'pawn outer')
        pc = req(mem, supplied + 0x250, '<Q', 'pawn controller')
        base = req(mem, PAWN_CLASS, '<Q', 'pawn base class')
        if not (level and pc and base):
            raise RuntimeError('pawn, controller, level, or pawn class unavailable')
        vt = req(mem, pc, '<Q', 'controller vtable')
        if not (RLO <= vt < RHI and one(mem, vt + SLOT, '<Q') == DRIVER
                and one(mem, pc + 0xf0, '<B') == 2 and one(mem, pc + 0x220, '<Q')
                and one(mem, pc + 0x20, '<Q') == level
                and one(mem, pc + 0x248, '<Q') == supplied):
            raise RuntimeError('supplied pawn is not possessed by a validated local controller')
        addresses, counts = level_actors(mem, level)
        if supplied not in addresses or pc not in addresses:
            raise RuntimeError('controller or possessed pawn absent from current ULevel arrays')
        if not pawn_actor(mem, supplied, level, base):
            raise RuntimeError('possessed pawn failed class/outer validation')
        pawns = []
        for p in sorted(addresses):
            if not p or not pawn_actor(mem, p, level, base):
                continue
            mesh = req(mem, p + 0x278, '<Q', 'character mesh')
            current = req(mem, p + 0x588, '<Q', 'current weapon')
            attached = req(mem, p + 0x6d8, '<Q', 'attached weapon')
            weapon = None
            if current:
                if current not in addresses or one(mem, current + 0x20, '<Q') != level:
                    raise RuntimeError(f'current weapon {current:#x} not validated in level')
                root = req(mem, current + 0x130, '<Q', 'weapon root')
                data = req(mem, current + 0x350, '<Q', 'weapon data')
                weapon = {'object': obj(mem, current), 'weapon_data': obj(mem, data),
                          'hand_socket_override_fname_raw': req(mem, current + 0x9a8, '<II', 'weapon hand socket override'),
                          'owner': ptr(req(mem, current + 0xe0, '<Q', 'weapon owner')),
                          'instigator': ptr(req(mem, current + 0x118, '<Q', 'weapon instigator')),
                          'overlay_class': obj(mem, req(mem, current + 0xac0, '<Q', 'weapon overlay class')),
                          'root': component(mem, root), 'root_parent_is_mesh': bool(root and one(mem, root + 0xc0, '<Q') == mesh),
                          'equipping': bool(req(mem, current + 0x288, '<B', 'weapon flags') & 1)}
            anim = req(mem, mesh + 0x630, '<Q', 'animation instance') if mesh else 0
            asset = req(mem, mesh + 0x410, '<Q', 'skeletal mesh asset') if mesh else 0
            pawns.append({'object': obj(mem, p), 'own': p == supplied,
                          'hand_socket_fname_raw': req(mem, p + 0x5b8, '<II', 'pawn hand socket'),
                          'left_hand_socket_fname_raw': req(mem, p + 0x5c0, '<II', 'pawn left hand socket'),
                          'root': component(mem, req(mem, p + 0x130, '<Q', 'pawn root')),
                          'mesh': component(mem, mesh), 'animation_instance': obj(mem, anim),
                          'skeletal_mesh_asset': obj(mem, asset), 'current_weapon': weapon,
                          'overlay_class': obj(mem, req(mem, p + 0xa00, '<Q', 'pawn overlay class')),
                          'remote_view_data32_raw': req(mem, p + 0x2670, '<I', 'remote view data'),
                          'remote_view_pitch_raw': req(mem, p + 0x22a, '<B', 'remote view pitch'),
                          'attached_weapon': ptr(attached), 'current_equals_attached': bool(current and current == attached),
                          'flags': {k: bool(req(mem, p + o, '<B', k) & mask) for k, (o, mask) in FLAGS.items()},
                          'current_synced_montage': obj(mem, req(mem, p + 0x1158, '<Q', 'synced montage'))})
        after, _ = level_actors(mem, level)
        if (one(mem, pc + 0x248, '<Q') != supplied or one(mem, supplied + 0x250, '<Q') != pc
                or supplied not in after or pc not in after):
            raise RuntimeError('possession or level membership changed during capture')
        return {'pid': pid, 'controller': ptr(pc), 'level': ptr(level), 'own_pawn': ptr(supplied),
                'level_array_counts': counts, 'pawns': pawns, 'start_unix_ns': started, 'end_unix_ns': time.time_ns()}
    finally:
        os.close(mem)


def client_arg(value):
    try:
        if ':' in value:
            pid, pawn = value.split(':', 1)
            result = int(pid), int(pawn, 0)
            if min(result) <= 0: raise ValueError()
            return result
        pid = int(value)
        if pid <= 0: raise ValueError()
        return pid, None
    except ValueError:
        raise argparse.ArgumentTypeError('expected PID or PID:PAWN_ADDRESS (address may be hex)')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('clients', nargs='+', type=client_arg)
    parser.add_argument('--output', required=True, type=Path, help='JSON file; replaced atomically only after complete validation')
    args = parser.parse_args()
    try:
        result = {'schema': 'equipment-pose-sampler-v1', 'clients': [snapshot(*client) for client in args.clients],
                  'limitations': ['Sequential, non-atomic reads; no socket world transform, bone pose, or animation graph state measured.',
                                  'FName pairs are raw process-local indices; no validated live decoder available.']}
        data = json.dumps(result, indent=2, allow_nan=False) + '\n'
        temp = None
        try:
            with tempfile.NamedTemporaryFile(mode='w', encoding='utf-8', dir=args.output.parent, prefix='.equipment-pose-', delete=False) as f:
                temp = f.name
                f.write(data); f.flush(); os.fsync(f.fileno())
            os.replace(temp, args.output); temp = None
        finally:
            if temp is not None: os.unlink(temp)
    except (OSError, RuntimeError, ValueError) as exc:
        parser.exit(1, f'capture failed: {exc}\n')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
