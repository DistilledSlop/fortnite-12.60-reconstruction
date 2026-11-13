#!/usr/bin/env python3
"""Read-only original 12.60 client phase snapshot, supplied as PID:current-pawn.

Offsets: GameState fields from reflection/name_tables/struct_layouts.json; aircraft
DropStart/End from original getters 0x1418c3790/0x1418c3780; server clock from
GameState vtable +0x630, original 0x144216b40 (world+0x540 + state+0x248).
World and local controller are discovered from the validated GEngine viewport
chain by probe_equipment_pose.discover_context; no possession is required.
"""
import argparse
import json
import math
import os
import struct
from pathlib import Path
import tempfile
import time

from probe_equipment_pose import (DRIVER, PAWN_CLASS, RHI, RLO, SLOT, client_arg,
                                  discover_context, level_actors, obj, one, pawn_actor, ptr, req)

GS_CLASS = 0x148199598  # Original AFortGameStateAthena StaticClass 0x14290f690.
CLOCK_GETTER = 0x144216b40


def finite(mem, address, label):
    value = req(mem, address, '<f', label)
    if not math.isfinite(value):
        raise RuntimeError(f'nonfinite {label}')
    return value


def snapshot(pid, pawn):
    started = time.time_ns()
    context = discover_context(pid)
    world, level, pc = (context[k] for k in ('world', 'level', 'pc'))
    current_pawn = context['pawn'] or 0
    if pawn is not None and pawn != current_pawn:
        raise RuntimeError('supplied pawn is not currently possessed by the local controller')
    mem = os.open(f'/proc/{pid}/mem', os.O_RDONLY)
    try:
        gs_class = req(mem, GS_CLASS, '<Q', 'Athena GameState base class')
        if not all((world, level, pc, gs_class)):
            raise RuntimeError('world, controller, level or GameState class unavailable')
        vt = req(mem, pc, '<Q', 'controller vtable')
        if not (RLO <= vt < RHI and one(mem, vt + SLOT, '<Q') == DRIVER
                and one(mem, pc + 0xf0, '<B') == 2 and one(mem, pc + 0x220, '<Q')
                and one(mem, pc + 0x20, '<Q') == level
                and (one(mem, pc + 0x248, '<Q') or 0) == current_pawn
                and one(mem, world + 0x30, '<Q') == level):
            raise RuntimeError('world or local controller failed validation')
        actors, counts = level_actors(mem, level)
        if pc not in actors:
            raise RuntimeError('local controller absent from level')
        if current_pawn:
            pawn_class = req(mem, PAWN_CLASS, '<Q', 'pawn base class')
            if (not pawn_class or current_pawn not in actors
                    or not pawn_actor(mem, current_pawn, level, pawn_class)
                    or one(mem, current_pawn + 0x250, '<Q') != pc):
                raise RuntimeError('current pawn failed possession, level or class validation')
        gs = req(mem, world + 0x130, '<Q', 'world GameState')
        if not gs or gs not in actors or not pawn_actor(mem, gs, level, gs_class):
            raise RuntimeError('world GameState failed level or Athena class validation')
        gvt = one(mem, gs, '<Q')
        if not gvt or not RLO <= gvt < RHI or one(mem, gvt + 0x630, '<Q') != CLOCK_GETTER:
            raise RuntimeError('GameState clock getter did not match original client')
        obj(mem, world)
        world_time = finite(mem, world + 0x540, 'world time')
        delta = finite(mem, gs + 0x248, 'server clock delta')
        phase = req(mem, gs + 0x1cb0, '<B', 'game phase')
        step = req(mem, gs + 0x1eba, '<B', 'game phase step')
        if phase > 5 or step > 11:
            raise RuntimeError('invalid game phase or step')
        data, count, capacity = req(mem, gs + 0x1ee0, '<Qii', 'aircraft array')
        if not (0 <= count <= capacity <= 1024 and (data or count == 0)):
            raise RuntimeError('invalid aircraft array')
        aircraft = []
        aircraft_addresses = []
        for index in range(count):
            address = req(mem, data + index * 8, '<Q', 'aircraft entry')
            aircraft_addresses.append(address)
            if not address:
                aircraft.append(None)  # unresolved replicated actor reference is meaningful
                continue
            if address not in actors or one(mem, address + 0x20, '<Q') != level:
                raise RuntimeError('aircraft actor not in validated level')
            aircraft.append({'object': obj(mem, address),
                             'drop_start_time': finite(mem, address + 0x2d8, 'drop start'),
                             'drop_end_time': finite(mem, address + 0x2dc, 'drop end')})
        result = {'pid': pid, 'pawn': ptr(current_pawn), 'controller': ptr(pc), 'level': ptr(level),
                  'world': ptr(world), 'game_state': obj(mem, gs), 'level_array_counts': counts,
                  'game_phase': phase, 'game_phase_step': step,
                  'game_phase_step_time_remaining': finite(mem, gs + 0x1ebc, 'step time'),
                  'warmup_start_time': finite(mem, gs + 0x12d4, 'warmup start'),
                  'warmup_end_time': finite(mem, gs + 0x12d8, 'warmup end'),
                  'world_time_seconds': world_time, 'server_world_time_delta': delta,
                  'server_world_time_seconds': struct.unpack('<f', struct.pack('<f', world_time + delta))[0], 'aircraft': aircraft,
                  'start_unix_ns': started, 'end_unix_ns': time.time_ns()}
        after, _ = level_actors(mem, level)
        after_context = discover_context(pid)
        if (any(after_context[k] != context[k] for k in ('world', 'level', 'pc'))
                or (after_context['pawn'] or 0) != current_pawn
                or (one(mem, pc + 0x248, '<Q') or 0) != current_pawn
                or (current_pawn and (current_pawn not in after
                    or one(mem, current_pawn + 0x250, '<Q') != pc))
                or pc not in after or gs not in after
                or one(mem, world + 0x30, '<Q') != level or one(mem, world + 0x130, '<Q') != gs
                or req(mem, gs + 0x1ee0, '<Qii', 'aircraft array recheck') != [data, count, capacity]
                or any(one(mem, data + i * 8, '<Q') != address
                       for i, address in enumerate(aircraft_addresses))):
            raise RuntimeError('possession, world or aircraft array changed during capture')
        return result
    finally:
        os.close(mem)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('clients', nargs='+', type=client_arg)
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args()
    try:
        result = {'schema': 'athena-phase-snapshot-v1', 'clients': [snapshot(*c) for c in args.clients],
                  'limitations': ['Sequential reads are not an atomic game tick.',
                                  'Capture fails if the local pawn, world or controller changes during the sequential reads.']}
        data = json.dumps(result, indent=2, allow_nan=False) + '\n'
        temp = None
        try:
            with tempfile.NamedTemporaryFile(mode='w', encoding='utf-8', dir=args.output.parent,
                                             prefix='.athena-phase-', delete=False) as f:
                temp = f.name
                f.write(data)
                f.flush()
                os.fsync(f.fileno())
            os.replace(temp, args.output)
            temp = None
        finally:
            if temp is not None:
                os.unlink(temp)
    except (OSError, RuntimeError, ValueError, OverflowError, struct.error) as exc:
        parser.exit(1, f'phase probe failed: {exc}\n')


if __name__ == '__main__':
    main()
