#!/usr/bin/env python3
"""Read-only original 12.60 live health bar cache snapshot; usage: script PID."""
import argparse
import json
import math
import os
import struct
import sys
import time

from probe_equipment_pose import discover_context, obj, one, ptr, req

# GPSC cache operands from 1451ae660, 1451aa100, 14519a3e0 respectively.
CLASS_SLOTS = {'bar': 0x1483a2a78, 'local_vm': 0x1483a1c00, 'hud_context': 0x14839f4e8}
PLAYER_STATE_CLASS_SLOT = 0x148361bd0  # GPSC 0x1447c8410
CHUNK_TABLE, ELEMENT_COUNT = 0x148263a18, 0x148263a2c
ITEM_SIZE, PER_CHUNK = 24, 65536
DEAD = 0x30000000  # FUObjectItem Unreachable | PendingKill
TEMPLATE = 0x30  # RF_ClassDefaultObject | RF_ArchetypeObject


def snapshot(pid):
    context = discover_context(pid)
    start = time.time_ns()
    mem = os.open(f'/proc/{pid}/mem', os.O_RDONLY)
    try:
        table = req(mem, CHUNK_TABLE, '<Q', 'object chunk table')
        count = req(mem, ELEMENT_COUNT, '<i', 'object count')
        if not table or not 0 < count <= 4_000_000:
            raise RuntimeError('invalid object registry table/count')
        slots = {k: req(mem, a, '<Q', k + ' class slot') for k, a in CLASS_SLOTS.items()}
        if not slots['bar']:
            raise RuntimeError('bar class has not been registered')
        for address in filter(None, slots.values()):
            obj(mem, address)
        chunks = (count + PER_CHUNK - 1) // PER_CHUNK
        chunk_ptrs = req(mem, table, '<' + str(chunks) + 'Q', 'chunk pointers')
        if chunks == 1:
            chunk_ptrs = [chunk_ptrs]
        registry = {}
        entries = []
        for ci, chunk in enumerate(chunk_ptrs):
            n = min(PER_CHUNK, count - ci * PER_CHUNK)
            if not chunk:
                raise RuntimeError(f'null object chunk {ci}')
            data = os.pread(mem, n * ITEM_SIZE, chunk)
            if len(data) != n * ITEM_SIZE:
                raise RuntimeError(f'unreadable object chunk {ci}')
            for j in range(n):
                address, flags = struct.unpack_from('<QI', data, j * ITEM_SIZE)
                if address and not flags & DEAD:
                    if address in registry:
                        raise RuntimeError('duplicate live object pointer in registry')
                    registry[address] = ci * PER_CHUNK + j
                    entries.append((address, ci * PER_CHUNK + j))
        for k, address in slots.items():
            if address and address not in registry:
                raise RuntimeError(f'{k} class missing from object registry')
        base_depths = {}
        for k, base in slots.items():
            if base:
                depth = req(mem, base + 0x38, '<i', k + ' base depth')
                if not 0 <= depth < 256:
                    raise RuntimeError(f'invalid {k} class depth')
                base_depths[k] = depth
        class_cache = {}
        targets = {k: [] for k in slots}
        templates = {k: 0 for k in slots}
        for address, index in entries:
            # UObject header: vtable, flags, index, class, name, outer.
            header = os.pread(mem, 0x28, address)
            if len(header) != 0x28:
                raise RuntimeError(f'unreadable registered object {address:#x}')
            vt, flags, actual_index, cls = struct.unpack_from('<QI i Q', header)
            if actual_index != index or cls not in registry:
                raise RuntimeError(f'object registry changed or invalid header at {address:#x}')
            if cls not in class_cache:
                depth = req(mem, cls + 0x38, '<i', 'class depth')
                ancestry = req(mem, cls + 0x30, '<Q', 'class ancestry')
                if not 0 <= depth < 256 or not ancestry:
                    raise RuntimeError(f'invalid inheritance layout for {cls:#x}')
                class_cache[cls] = {k for k, base in slots.items() if base and
                    (bd := base_depths[k]) <= depth and
                    one(mem, ancestry + bd * 8, '<Q') == base + 0x30}
            for kind in class_cache[cls]:
                if flags & TEMPLATE:
                    templates[kind] += 1
                else:
                    targets[kind].append(address)
        def registered(address, label):
            if address and address not in registry:
                raise RuntimeError(f'{label} missing from live registry: {address:#x}')
            return obj(mem, address)
        bars = []
        for address in targets['bar']:
            vm = req(mem, address + 0x240, '<Q', 'bar PlayerVM')
            values = [req(mem, address + off, '<f', label) for off, label in
                      ((0x24c, 'current'), (0x258, 'last'), (0x25c, 'max'))]
            if not all(math.isfinite(x) for x in values):
                raise RuntimeError(f'non-finite bar value at {address:#x}')
            if vm and vm not in targets['local_vm']:
                # The base type permits nonlocal PlayerVMs. Identity is still useful;
                # only read +1d0 when the concrete local type is proven.
                registered(vm, 'bar VM')
            bars.append({'object': registered(address, 'bar'), 'outer': ptr(req(mem, address + 0x20, '<Q', 'bar outer')),
                         'bar_type_raw': req(mem, address + 0x248, '<B', 'bar type'),
                         'player_vm': registered(vm, 'bar VM'), 'current': values[0], 'last': values[1], 'max': values[2]})
        vms = []
        for address in targets['local_vm']:
            pc = req(mem, address + 0x1d0, '<Q', 'VM controller')
            vm_vtable = req(mem, address, '<Q', 'VM vtable')
            vms.append({'object': registered(address, 'VM'),
                        'player_state_virtual_260': ptr(req(mem, vm_vtable + 0x260, '<Q', 'VM virtual 260')),
                        'player_controller': registered(pc, 'VM controller'),
                        'is_local_controller': pc == context['pc']})
        huds = []
        for address in targets['hud_context']:
            pawn = req(mem, address + 0x250, '<Q', 'HUD bound pawn')
            vm = req(mem, address + 0x258, '<Q', 'HUD VM')
            huds.append({'object': registered(address, 'HUD context'),
                         'outer': registered(req(mem, address + 0x20, '<Q', 'HUD context outer'), 'HUD context outer'),
                         'bound_pawn': registered(pawn, 'HUD bound pawn'),
                         'player_vm': registered(vm, 'HUD VM'), 'is_local_pawn': bool(pawn and pawn == context['pawn'])})
        for kind, addresses in targets.items():
            for address in addresses:
                index = registry[address]
                item = chunk_ptrs[index // PER_CHUNK] + (index % PER_CHUNK) * ITEM_SIZE
                item_address, item_flags = req(mem, item, '<QI', 'object item recheck')
                if (item_address != address or item_flags & DEAD or
                    req(mem, address + 0xc, '<i', 'object index recheck') != index or
                    req(mem, address + 8, '<I', 'object flags recheck') & TEMPLATE):
                    raise RuntimeError(f'{kind} object changed during capture; retry')
        if (req(mem, CHUNK_TABLE, '<Q', 'chunk table recheck') != table or
            req(mem, ELEMENT_COUNT, '<i', 'object count recheck') != count or
            req(mem, context['pc'] + 0x248, '<Q', 'possessed pawn recheck') != context['pawn'] or
            any(req(mem, CLASS_SLOTS[k], '<Q', k + ' recheck') != v for k, v in slots.items())):
            raise RuntimeError('context or registry changed during capture; retry')
        pawn = context['pawn']
        pawn_vtable = req(mem, pawn, '<Q', 'pawn vtable') if pawn else 0
        getter_slots = {name: ptr(req(mem, pawn_vtable + off, '<Q', name + ' slot'))
                        for name, off in (('max_health', 0xb20), ('health', 0xb28),
                                          ('max_shield', 0xb30), ('shield', 0xb38))} if pawn else None
        pawn_bits = {'byte_58': req(mem, pawn + 0x58, '<B', 'pawn flags 58'),
                     'byte_539': req(mem, pawn + 0x539, '<B', 'pawn flags 539')} if pawn else None
        state_base = req(mem, PLAYER_STATE_CLASS_SLOT, '<Q', 'PlayerState class slot')
        registered(state_base, 'PlayerState class')
        if not state_base:
            raise RuntimeError('PlayerState class unavailable')
        state_depth = req(mem, state_base + 0x38, '<i', 'PlayerState class depth')
        if not 0 <= state_depth < 256:
            raise RuntimeError('invalid PlayerState class depth')
        def validate_state(address):
            registered(address, 'PlayerState')
            if address:
                cls = req(mem, address + 0x10, '<Q', 'PlayerState class')
                depth = req(mem, cls + 0x38, '<i', 'state class depth')
                ancestry = req(mem, cls + 0x30, '<Q', 'state ancestry')
                if not (state_depth <= depth < 256 and ancestry and
                        one(mem, ancestry + state_depth * 8, '<Q') == state_base + 0x30):
                    raise RuntimeError('invalid PlayerState type')
        pc_state = req(mem, context['pc'] + 0x220, '<Q', 'controller PlayerState')
        validate_state(pc_state)
        state_pawn = req(mem, pc_state + 0x278, '<Q', 'controller PlayerState PawnPrivate') if pc_state else 0
        pawn_state = req(mem, pawn + 0x238, '<Q', 'pawn PlayerState') if pawn else 0
        validate_state(pawn_state)
        pawn_state_pawn = req(mem, pawn_state + 0x278, '<Q', 'pawn PlayerState PawnPrivate') if pawn_state else 0
        pc_vtable = req(mem, context['pc'], '<Q', 'controller vtable')
        return {'pid': pid, 'start_ns': start, 'end_ns': time.time_ns(),
                'pawn_player_state': registered(pawn_state, 'pawn PlayerState'),
                'pawn_player_state_pawn_private': registered(pawn_state_pawn, 'pawn PlayerState PawnPrivate'),
                'pawn_state_pawn_matches_possessed': bool(pawn_state_pawn and pawn_state_pawn == pawn),
                'controller_pawn_virtual_698': ptr(req(mem, pc_vtable + 0x698, '<Q', 'controller virtual 698')),
                'controller_player_state_pawn_private': registered(state_pawn, 'PlayerState PawnPrivate'),
                'state_pawn_matches_possessed': bool(state_pawn and state_pawn == pawn),
                'pawn_virtual_getter_slots': getter_slots, 'pawn_getter_flag_bytes': pawn_bits,
                'controller_player_state': registered(pc_state, 'controller PlayerState'),
                'controller': registered(context['pc'], 'local controller'),
                'pawn': registered(context['pawn'], 'local pawn'), 'registry_count': count,
                'template_counts_excluded': templates, 'bars': bars, 'local_view_models': vms, 'hud_contexts': huds,
                'limits': ['FName pairs are raw process-local indices; bar_type is an undecoded byte.',
                           'Registry-wide snapshot is not atomic; retry during transitions. Cache values do not establish displayed text.']}
    finally:
        os.close(mem)


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('pid', type=int)
    args = ap.parse_args()
    try:
        print(json.dumps(snapshot(args.pid), indent=2, allow_nan=False))
    except (OSError, RuntimeError, ValueError, struct.error) as exc:
        print(f'health HUD probe failed: {exc}', file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
