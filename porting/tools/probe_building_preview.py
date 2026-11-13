#!/usr/bin/env python3
"""Read-only original 12.60 client building snapshot. Supply one or more PIDs."""
import argparse
import json
import os
from pathlib import Path
import tempfile
import time

from probe_equipment_pose import (discover_context, level_actors, obj, one, ptr,
                                  req, pawn_actor, PAWN_CLASS)

# StaticClass cache addresses: RIP-relative loads in original accessors
# 0x142927a60, 0x142759920, 0x14274a050, 0x142752f80, respectively.
ZONE_CLASS = 0x14819b7d8
GRID_CLASS = 0x1481781f8
PREVIEW_CLASS = 0x148177148
BUILDING_CLASS = 0x148177e28


def is_a(mem, cls, base):
    if not cls or not base:
        return False
    depth = one(mem, cls + 0x38, '<i')
    base_depth = one(mem, base + 0x38, '<i')
    table = one(mem, cls + 0x30, '<Q')
    return (depth is not None and base_depth is not None and
            0 <= base_depth <= depth < 256 and bool(table) and
            one(mem, table + base_depth * 8, '<Q') == base + 0x30)


def typed(mem, address, base, label):
    info = obj(mem, address)
    if not is_a(mem, req(mem, address + 0x10, '<Q', label + ' class'), base):
        raise RuntimeError(f'{label} {address:#x} failed class validation')
    return info


def actor(mem, address, base, label, level, actors):
    if address not in actors or one(mem, address + 0x20, '<Q') != level:
        raise RuntimeError(f'{label} {address:#x} failed level membership')
    return typed(mem, address, base, label)


def snapshot(pid):
    started = time.time_ns()
    context = discover_context(pid)
    mem = os.open(f'/proc/{pid}/mem', os.O_RDONLY)
    try:
        pc, pawn, world, level = (context[k] for k in ('pc', 'pawn', 'world', 'level'))
        actors, counts = level_actors(mem, level)
        if pc not in actors or not pawn or pawn not in actors:
            raise RuntimeError('controller or possessed pawn absent from level')
        pawn_base = req(mem, PAWN_CLASS, '<Q', 'pawn class cache')
        if not pawn_actor(mem, pawn, level, pawn_base) or one(mem, pawn + 0x250, '<Q') != pc:
            raise RuntimeError('pawn class or possession invalid')
        bases = {k: req(mem, v, '<Q', k + ' class cache') for k, v in
                 [('zone', ZONE_CLASS), ('grid', GRID_CLASS), ('preview', PREVIEW_CLASS), ('building', BUILDING_CLASS)]}
        if not all(bases.values()):
            raise RuntimeError('required original class cache is uninitialized')
        for base in bases.values():
            obj(mem, base)

        # World +0x130 and zone +0xb60: original 0x141c77490 / 0x141e9e770.
        gs = req(mem, world + 0x130, '<Q', 'world game state')
        gs_info = obj(mem, gs) if gs else None
        zone = bool(gs and is_a(mem, req(mem, gs + 0x10, '<Q', 'game state class'), bases['zone']))
        state = {'object': gs_info, 'is_fort_game_state_zone': zone}
        grid = 0
        if zone:
            if gs not in actors or one(mem, gs + 0x20, '<Q') != level:
                raise RuntimeError('game state failed level membership')
            grid = req(mem, gs + 0xb60, '<Q', 'structural support system')
            grid_info = typed(mem, grid, bases['grid'], 'structural support system') if grid else None
            if grid and one(mem, grid + 0x20, '<Q') != gs:
                raise RuntimeError('structural support system outer is not game state')
            # 0x141ea12b0: max != -1 && max <= total; not the overall result.
            total = req(mem, gs + 0x9ec, '<i', 'total structures')
            maximum = req(mem, gs + 0x9f0, '<i', 'max structures')
            state.update({'total_player_structures': total, 'max_player_structures': maximum,
                          'structure_cap_predicate': maximum != -1 and maximum <= total,
                          'support_system': grid_info,
                          'layout_allow_at_requirements': bool(req(mem, gs + 0xe14, '<B', 'layout at') & 1),
                          'layout_allow_without_requirements': bool(req(mem, gs + 0xe15, '<B', 'layout without') & 1),
                          'layout_feature_enabled': bool(req(mem, gs + 0xe16, '<B', 'layout feature') & 1)})
            if grid:
                # 0x141c78270: origin and bounds, not a result-4 condition.
                state['grid_origin'] = req(mem, grid + 0x378, '<3f', 'grid origin')
                state['grid_min'] = req(mem, grid + 0x384, '<3f', 'grid minimum')
                state['grid_max'] = req(mem, grid + 0x390, '<3f', 'grid maximum')

        # Reflection fields; bool masks from generated/reflection/bool_bits.json.
        build_class = req(mem, pc + 0x1350, '<Q', 'current buildable class')
        class_info = obj(mem, build_class) if build_class else None
        if build_class and not is_a(mem, build_class, bases['building']):
            raise RuntimeError('current buildable class is not a BuildingSMActor class')
        marker = req(mem, pc + 0x1190, '<Q', 'preview marker')
        extra = req(mem, pc + 0x1198, '<Q', 'extra preview marker')
        weapon = req(mem, pawn + 0x588, '<Q', 'current weapon')
        if weapon and (weapon not in actors or one(mem, weapon + 0x20, '<Q') != level):
            raise RuntimeError('current weapon failed level membership')
        weapon_info = obj(mem, weapon) if weapon else None
        tool = {'object': weapon_info}
        if weapon:
            tool.update({'owner': ptr(req(mem, weapon + 0xe0, '<Q', 'weapon owner')),
                         'instigator': ptr(req(mem, weapon + 0x118, '<Q', 'weapon instigator')),
                         'weapon_data': obj(mem, req(mem, weapon + 0x350, '<Q', 'weapon data'))})
        controller = {'object': obj(mem, pc), 'pawn': obj(mem, pawn),
                      'player_state': obj(mem, req(mem, pc + 0x220, '<Q', 'player state')),
                      'current_buildable_class': class_info,
                      'preview_marker': actor(mem, marker, bases['preview'], 'preview marker', level, actors) if marker else None,
                      'preview_extra_piece': actor(mem, extra, bases['preview'], 'preview extra piece', level, actors) if extra else None,
                      'build_free': bool(req(mem, pc + 0x1344, '<B', 'build free') & 1),
                      'preview_mirrored': bool(req(mem, pc + 0x133c, '<B', 'preview mirrored') & 1),
                      'preview_rotation_iterations': req(mem, pc + 0x1338, '<B', 'preview rotation'),
                      'last_grid_snap_location': req(mem, pc + 0x1464, '<3f', 'last grid snap location'),
                      'last_grid_snap_rotation': req(mem, pc + 0x1470, '<3f', 'last grid snap rotation'),
                      'last_grid_snap_centroid': req(mem, pc + 0x147c, '<3f', 'last grid snap centroid'),
                      'current_weapon': tool}
        after, _ = level_actors(mem, level)
        if (one(mem, pc + 0x248, '<Q') != pawn or one(mem, pawn + 0x250, '<Q') != pc
                or one(mem, world + 0x130, '<Q') != gs or (zone and one(mem, gs + 0xb60, '<Q') != grid)
                or one(mem, pawn + 0x588, '<Q') != weapon or one(mem, pc + 0x1350, '<Q') != build_class
                or one(mem, pc + 0x1190, '<Q') != marker or one(mem, pc + 0x1198, '<Q') != extra
                or any(a and a not in after for a in (pc, pawn, gs if zone else 0, marker, extra, weapon))):
            raise RuntimeError('capture context changed during read')
        return {'pid': pid, 'world': obj(mem, world), 'level': ptr(level), 'level_array_counts': counts,
                'game_state': state, 'controller': controller,
                'start_unix_ns': started, 'end_unix_ns': time.time_ns()}
    finally:
        os.close(mem)


def pid_arg(value):
    try:
        pid = int(value)
        if pid <= 0:
            raise ValueError()
        return pid
    except ValueError:
        raise argparse.ArgumentTypeError('expected positive PID')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('clients', nargs='+', type=pid_arg)
    parser.add_argument('--output', required=True, type=Path, help='JSON file; replaced atomically after validation')
    args = parser.parse_args()
    try:
        result = {'schema': 'building-preview-sampler-v1', 'clients': [snapshot(pid) for pid in args.clients],
                  'limitations': ['Sequential reads are not an atomic game-frame snapshot; stable pointers do not guarantee stable scalar fields.',
                                  'Placement result, class CDO, Athena predicates, location tags, collision and support outcomes are not measured; cap predicate alone is not the final decision.',
                                  'Current weapon type and marker color are not decoded; FName pairs are raw process-local indices.']}
        data = json.dumps(result, indent=2, allow_nan=False) + '\n'
        temp = None
        try:
            with tempfile.NamedTemporaryFile(mode='w', encoding='utf-8', dir=args.output.parent, prefix='.building-preview-', delete=False) as f:
                temp = f.name
                f.write(data)
                f.flush()
                os.fsync(f.fileno())
            os.replace(temp, args.output)
            temp = None
        finally:
            if temp is not None:
                os.unlink(temp)
    except (OSError, RuntimeError, ValueError) as exc:
        parser.exit(1, f'capture failed: {exc}\n')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
