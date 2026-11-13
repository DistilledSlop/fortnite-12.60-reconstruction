#!/usr/bin/env python3
"""Read original client aircraft state; supply PID or PID:COMPONENT_ADDRESS.

Original GetAircraftComponent 0x141695900 calls controller virtual +0x610
(0x143f709a0), which searches the owned component sparse set at +0x198.
The probe validates the controller outer, class and original vtable slots.
Output is a sequential snapshot, not an atomic tick or an RPC trace.
"""
import argparse
import json
import math
import os
from pathlib import Path
import tempfile
import time

from probe_equipment_pose import (DRIVER, FLAGS, PAWN_CLASS, RHI, RLO, SLOT,
                                  component, discover_context, level_actors, obj,
                                  one, pawn_actor, ptr, req)


def find_aircraft_component(mem, pc):
    # Original GetComponentByClass 0x143f709a0 iterates this sparse set:
    # +0x198 TArray of 16-byte set elements; +0x1a8 inline allocation bits,
    # optional heap allocation pointer +0x1b8, number of bits +0x1c0.
    base = req(mem, 0x1481908a8, '<Q', 'aircraft component StaticClass')
    if not base:
        raise RuntimeError('aircraft component class unavailable')
    depth = req(mem, base + 0x38, '<i', 'aircraft class depth')
    data, count, capacity = req(mem, pc + 0x198, '<Qii', 'owned component set')
    external = req(mem, pc + 0x1b8, '<Q', 'allocation bit storage')
    nbits = req(mem, pc + 0x1c0, '<i', 'allocation bit count')
    if not (0 <= depth < 256 and 0 <= count <= capacity <= 16384
            and nbits == count and (data or count == 0)
            and (external or nbits <= 128)):
        raise RuntimeError('invalid owned component sparse set')
    bitaddr = external or pc + 0x1a8
    words = [req(mem, bitaddr + i * 4, '<I', 'allocation bits')
             for i in range((nbits + 31) // 32)]
    matches = []
    for i in range(count):
        if not words[i // 32] & (1 << (i % 32)):
            continue
        item = req(mem, data + 16 * i, '<Q', 'owned component entry')
        if not item:
            raise RuntimeError('null allocated component')
        cls = req(mem, item + 0x10, '<Q', 'component class')
        if not cls:
            raise RuntimeError('component class unavailable')
        objdepth = req(mem, cls + 0x38, '<i', 'component class depth')
        table = req(mem, cls + 0x30, '<Q', 'component class ancestry')
        if 0 <= depth <= objdepth < 256 and table and one(mem, table + depth * 8, '<Q') == base + 0x30:
            matches.append(item)
    if len(matches) != 1:
        raise RuntimeError(f'expected one aircraft component, got {len(matches)}')
    if (req(mem, pc + 0x198, '<Qii', 'owned set recheck') != [data, count, capacity]
            or req(mem, pc + 0x1b8, '<Q', 'bit storage recheck') != external
            or req(mem, pc + 0x1c0, '<i', 'bit count recheck') != nbits
            or words != [req(mem, bitaddr + i * 4, '<I', 'bit recheck') for i in range(len(words))]):
        raise RuntimeError('owned component set changed during discovery')
    return matches[0]


def snapshot(pid, aircraft_component, expected_pawn):
    start = time.time_ns()
    ctx = discover_context(pid)
    world, level, pc, pawn = (ctx[k] for k in ('world', 'level', 'pc', 'pawn'))
    if expected_pawn is not None and pawn != expected_pawn:
        raise RuntimeError('supplied pawn is not currently possessed')
    mem = os.open(f'/proc/{pid}/mem', os.O_RDONLY)
    try:
        actors, counts = level_actors(mem, level)
        vt = req(mem, pc, '<Q', 'controller vtable')
        if not (pc in actors and RLO <= vt < RHI and one(mem, vt + SLOT, '<Q') == DRIVER
                and one(mem, pc + 0xf0, '<B') == 2 and one(mem, pc + 0x220, '<Q')
                and one(mem, pc + 0x20, '<Q') == level
                and (one(mem, pc + 0x248, '<Q') or 0) == (pawn or 0)
                and one(mem, world + 0x30, '<Q') == level):
            raise RuntimeError('local controller or world failed validation')
        c = find_aircraft_component(mem, pc)
        if aircraft_component is not None and c != aircraft_component:
            raise RuntimeError('supplied component is not the owned aircraft component')
        cobj = obj(mem, c)
        cvt = req(mem, c, '<Q', 'aircraft component vtable')
        if (one(mem, c + 0x20, '<Q') != pc or
                one(mem, cvt + 0x438, '<Q') != 0x141cce7c0 or
                one(mem, cvt + 0x430, '<Q') != 0x141cce7d0):
            raise RuntimeError('aircraft component owner or original vtable slots failed validation')
        aircraft = req(mem, c + 0xc0, '<Q', 'current aircraft')
        if aircraft and (aircraft not in actors or one(mem, aircraft + 0x20, '<Q') != level):
            raise RuntimeError('current aircraft is not a validated level actor')
        state = {'object': cobj, 'current_aircraft': obj(mem, aircraft),
                 'entered_byte_f8': req(mem, c + 0xf8, '<B', 'entered byte'),
                 'exited_byte_f9': req(mem, c + 0xf9, '<B', 'exited byte')}
        pawn_state = None
        root = movement = 0
        if pawn:
            base = req(mem, PAWN_CLASS, '<Q', 'pawn base class')
            if (not base or pawn not in actors or not pawn_actor(mem, pawn, level, base)
                    or one(mem, pawn + 0x250, '<Q') != pc):
                raise RuntimeError('possessed pawn failed class, level or possession validation')
            root = req(mem, pawn + 0x130, '<Q', 'pawn root')
            movement = req(mem, pawn + 0x280, '<Q', 'character movement')
            mv = None
            if movement:
                mv_obj = obj(mem, movement)
                if one(mem, movement + 0x20, '<Q') != pawn or one(mem, movement + 0x148, '<Q') != pawn:
                    raise RuntimeError('movement component owner validation failed')
                velocity = req(mem, movement + 0xc4, '<3f', 'velocity')
                if not all(math.isfinite(x) for x in velocity):
                    raise RuntimeError('nonfinite velocity')
                mv = {'object': mv_obj, 'velocity': velocity,
                      'movement_mode': req(mem, movement + 0x168, '<B', 'movement mode'),
                      'custom_movement_mode': req(mem, movement + 0x169, '<B', 'custom movement mode')}
            if root and one(mem, root + 0x20, '<Q') != pawn:
                raise RuntimeError('pawn root owner validation failed')
            pawn_state = {'object': obj(mem, pawn), 'root': component(mem, root), 'movement': mv,
                          'flags': {k: bool(req(mem, pawn + o, '<B', k) & mask)
                                    for k, (o, mask) in FLAGS.items() if 'skydiv' in k or 'skydive' in k or 'parachute' in k}}
        after, _ = level_actors(mem, level)
        ctx2 = discover_context(pid)
        if (ctx2 != ctx or pc not in after or find_aircraft_component(mem, pc) != c
                or (pawn and (pawn not in after or
                one(mem, pawn + 0x250, '<Q') != pc or one(mem, pawn + 0x130, '<Q') != root or
                one(mem, pawn + 0x280, '<Q') != movement)) or
                one(mem, c + 0x20, '<Q') != pc or one(mem, c + 0xc0, '<Q') != aircraft or
                one(mem, c + 0xf8, '<B') != state['entered_byte_f8'] or
                one(mem, c + 0xf9, '<B') != state['exited_byte_f9'] or
                (aircraft and aircraft not in after)):
            raise RuntimeError('context, component state or possession changed during capture')
        return {'pid': pid, 'world': ptr(world), 'level': ptr(level), 'controller': ptr(pc),
                'level_array_counts': counts, 'aircraft_component': state, 'pawn': pawn_state,
                'start_unix_ns': start, 'end_unix_ns': time.time_ns()}
    finally:
        os.close(mem)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('client', help='PID or PID:aircraft-component-address (address may be hex)')
    p.add_argument('--pawn', type=lambda v: int(v, 0), help='require this currently possessed pawn')
    p.add_argument('--output', type=Path, required=True)
    a = p.parse_args()
    temp = None
    try:
        parts = a.client.split(':', 1)
        pid = int(parts[0])
        address = int(parts[1], 0) if len(parts) == 2 else None
        if pid <= 0 or (address is not None and address <= 0) or (a.pawn is not None and a.pawn <= 0):
            raise ValueError('PID and addresses must be positive')
        result = {'schema': 'aircraft-state-snapshot-v1', 'client': snapshot(pid, address, a.pawn),
                  'limitations': ['Reads are sequential and do not capture an atomic game tick.',
                                  'No RPC tracing; state changes during reads can invalidate a capture.',
                                  'Movement and transforms can change between reads.']}
        with tempfile.NamedTemporaryFile(mode='w', encoding='utf-8', dir=a.output.parent,
                                         prefix='.aircraft-state-', delete=False) as f:
            temp = f.name
            f.write(json.dumps(result, indent=2, allow_nan=False) + '\n')
            f.flush()
            os.fsync(f.fileno())
        os.replace(temp, a.output)
        temp = None
    except (OSError, RuntimeError, ValueError, OverflowError) as exc:
        p.exit(1, f'aircraft state probe failed: {exc}\n')
    finally:
        if temp is not None:
            os.unlink(temp)


if __name__ == '__main__':
    main()
