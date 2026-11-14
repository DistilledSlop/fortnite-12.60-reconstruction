#!/usr/bin/env python3
"""Stream paired read-only original-client phase and aircraft observations as JSONL.

Run with sufficient permission to read each client's /proc/PID/mem. Start while
both clients are in warmup; sequential observations are not an atomic tick.
"""
import argparse
import json
import math
import os
import struct
from pathlib import Path
import time

from probe_athena_phase import finite, snapshot
from probe_equipment_pose import component, discover_context, obj, pawn_actor, req

LIMITATIONS = [
    'Client and component reads are sequential, not an atomic game tick.',
    'Clock and distance differences describe sequential measurements, not causality or acceptance.',
    'Aircraft root components and world settings are read after the validated phase snapshot; reads are not atomic.',
]


def process_identity(pid):
    # The command name in stat may contain spaces and parentheses.
    stat = Path(f'/proc/{pid}/stat').read_text()
    fields = stat[stat.rfind(')') + 2:].split()
    if fields[0] in ('Z', 'X'):
        raise ProcessLookupError(f'process {pid} is no longer running')
    return fields[19]  # starttime, field 22


# WorldSettings reflected offsets and original StaticClass cache provenance:
# porting/generated/agents/flight-frame-clamp/REPORT.md.
SETTINGS_CLASS = 0x148365c30
SETTINGS_FIELDS = {'time_dilation': 0x2e0, 'matinee_time_dilation': 0x2e4,
                   'demo_play_time_dilation': 0x2e8, 'min_global_time_dilation': 0x2ec,
                   'max_global_time_dilation': 0x2f0, 'min_undilated_frame_time': 0x2f4,
                   'max_undilated_frame_time': 0x2f8}


def settings_capture(mem, world, level, cached):
    base = req(mem, SETTINGS_CLASS, '<Q', 'WorldSettings class cache')
    if not base:
        raise RuntimeError('WorldSettings class unavailable')
    # Cache the array slot, not only the actor: each later sample verifies actual
    # level membership without another whole-level scan. Reacquire on changes.
    if cached and cached[:3] == (world, level, base):
        _, _, _, offset, index, actor = cached
        array = req(mem, level + offset, '<Qii', 'settings level array')
        data, count, capacity = array
        if (0 <= index < count <= capacity <= 200000 and data and
                req(mem, data + 8 * index, '<Q', 'settings array entry') == actor and
                pawn_actor(mem, actor, level, base)):
            pass
        else:
            cached = None
    else:
        cached = None
    if not cached:
        matches = {}
        for offset in (0x98, 0xa8):
            data, count, capacity = req(mem, level + offset, '<Qii', 'settings level array')
            if not (0 <= count <= capacity <= 200000 and (data or count == 0)):
                raise RuntimeError('invalid settings level array')
            entries = req(mem, data, '<' + str(count) + 'Q', 'settings level entries') if count else []
            if count == 1:
                entries = [entries]
            for index, actor in enumerate(entries):
                if actor and pawn_actor(mem, actor, level, base):
                    matches[actor] = (offset, index)
        if len(matches) != 1:
            raise RuntimeError(f'expected one WorldSettings actor, found {len(matches)}')
        actor, (offset, index) = next(iter(matches.items()))
        cached = (world, level, base, offset, index, actor)
    data, count, capacity = req(mem, level + offset, '<Qii', 'settings level array recheck')
    if not (0 <= index < count <= capacity <= 200000 and data and
            req(mem, data + 8 * index, '<Q', 'settings array entry recheck') == actor and
            pawn_actor(mem, actor, level, base)):
        raise RuntimeError('WorldSettings actor changed during capture')
    result = {'object': obj(mem, actor)}
    result.update({name: finite(mem, actor + off, name) for name, off in SETTINGS_FIELDS.items()})
    # Original Fortnite WorldSettings effective-dilation override at +0x638
    # multiplies a fourth, unreflected factor at +0x3e4. Validate the dispatch
    # before reading that extension; the base engine actor lacks this field.
    vtable = req(mem, actor, '<Q', 'WorldSettings vtable')
    if req(mem, vtable + 0x638, '<Q', 'effective dilation getter') != 0x141fc0920:
        raise RuntimeError('WorldSettings effective dilation getter did not match original Fortnite')
    result['native_effective_dilation_factor_3e4'] = finite(mem, actor + 0x3e4, 'native dilation factor')
    if (req(mem, SETTINGS_CLASS, '<Q', 'WorldSettings class recheck') != base or
            req(mem, actor, '<Q', 'WorldSettings vtable recheck') != vtable or
            req(mem, level + offset, '<Qii', 'settings array final recheck') != [data, count, capacity] or
            req(mem, data + 8 * index, '<Q', 'settings entry final recheck') != actor or
            not pawn_actor(mem, actor, level, base)):
        raise RuntimeError('WorldSettings actor changed during capture')
    return result, cached


def capture(pid, settings_cache):
    result = snapshot(pid, None)
    mem = os.open(f'/proc/{pid}/mem', os.O_RDONLY)
    try:
        world, level = int(result['world'], 16), int(result['level'], 16)
        # Original reflected UGameplayStatics getters: exec 0x14470f9c0 ->
        # 0x1442165e0 reads +0x548; exec 0x1447100a0 -> 0x144217e80
        # reads +0x550. See generated/agents/phase-frame-sampler/REPORT.md.
        result['world_real_time_seconds'] = finite(mem, world + 0x548, 'world real time')
        result['world_delta_time_seconds'] = finite(mem, world + 0x550, 'world delta time')
        result['world_settings'], cached = settings_capture(mem, world, level, settings_cache.get(pid))
        context = discover_context(pid)
        if (context['world'] != world or context['level'] != level or
                context['pc'] != int(result['controller'], 16)):
            raise RuntimeError('world or controller changed after phase snapshot')
        settings_cache[pid] = cached
        for aircraft in result['aircraft']:
            if aircraft is None:
                continue
            actor = int(aircraft['object']['address'], 16)
            # Original reflection block 0x1462e4f90; FlightInfo struct
            # 0x1479873f0. See generated/agents/paired-phase-sampler/offset-evidence.json.
            aircraft['flight_timing'] = {
                'flight_start_time': finite(mem, actor + 0x2d0, 'flight start time'),
                'replicated_flight_timestamp': finite(mem, actor + 0x2e4, 'replicated flight timestamp'),
                'flight_elapsed_time': finite(mem, actor + 0x2e8, 'flight elapsed time'),
                'client_flight_timer_drift': finite(mem, actor + 0x2ec, 'client flight timer drift'),
                'flight_speed': finite(mem, actor + 0x2a8 + 0x18, 'flight speed'),
            }
            root = req(mem, actor + 0x130, '<Q', 'aircraft root')
            if not root:
                aircraft['root'] = None
                continue
            if req(mem, root + 0x20, '<Q', 'aircraft root owner') != actor:
                raise RuntimeError('aircraft root owner does not match actor')
            root_data = component(mem, root)
            position = root_data['component_world_translation']
            if not all(isinstance(v, (int, float)) and math.isfinite(v) for v in position):
                raise RuntimeError('aircraft root position is not finite')
            if (req(mem, actor + 0x130, '<Q', 'aircraft root recheck') != root or
                    req(mem, root + 0x20, '<Q', 'aircraft root owner recheck') != actor):
                raise RuntimeError('aircraft root changed during capture')
            aircraft['root'] = root_data
    finally:
        os.close(mem)
    result['end_unix_ns'] = time.time_ns()
    return result


def position(client):
    # Compare the first resolved aircraft only, recording which actor supplied it.
    for aircraft in client['aircraft']:
        if aircraft and aircraft.get('root'):
            return aircraft['object']['address'], aircraft['root']['component_world_translation']
    return None


def positive(value):
    number = float(value)
    if not math.isfinite(number) or number <= 0:
        raise argparse.ArgumentTypeError('must be a positive finite number')
    return number


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('pids', metavar='PID', type=int, nargs='+')
    parser.add_argument('--seconds', type=positive, required=True)
    parser.add_argument('--interval', type=positive, required=True)
    parser.add_argument('--output', type=Path, required=True, help='new JSONL file; created exclusively with mode 0600')
    args = parser.parse_args()
    if len(args.pids) < 2 or any(pid <= 0 for pid in args.pids) or len(set(args.pids)) != len(args.pids):
        parser.error('supply at least two distinct positive original client PIDs')
    try:
        identities = {pid: process_identity(pid) for pid in args.pids}
        fd = os.open(args.output, os.O_WRONLY | os.O_CREAT | os.O_EXCL, 0o600)
    except OSError as exc:
        parser.exit(1, f'sampler setup failed: {exc}\n')
    start_wall = time.time_ns()
    start = time.monotonic()
    deadline = start + args.seconds
    distances, clocks = [], []
    last_errors, last_report = {}, {}
    settings_cache = {}
    captured = 0
    stopped = 'duration elapsed'
    with os.fdopen(fd, 'w', encoding='utf-8', buffering=1) as out:
        def emit(record):
            out.write(json.dumps(record, allow_nan=False, separators=(',', ':')) + '\n')
            out.flush()

        emit({'type': 'metadata', 'schema': 'paired-client-phase-v1', 'pids': args.pids,
              'process_start_ticks': identities, 'start_unix_ns': start_wall,
              'seconds': args.seconds, 'interval': args.interval, 'limitations': LIMITATIONS})
        next_sample = start
        while time.monotonic() < deadline:
            delay = next_sample - time.monotonic()
            if delay > 0:
                time.sleep(min(delay, max(0, deadline - time.monotonic())))
            if time.monotonic() >= deadline:
                break
            cycle_start = time.time_ns()
            clients = []
            died = False
            for pid in args.pids:
                try:
                    if process_identity(pid) != identities[pid]:
                        raise ProcessLookupError(f'process {pid} identity changed')
                    client = capture(pid, settings_cache)
                    if process_identity(pid) != identities[pid]:
                        raise ProcessLookupError(f'process {pid} identity changed during capture')
                    clients.append(client)
                    last_errors.pop(pid, None)
                except (OSError, RuntimeError, ValueError, OverflowError, struct.error) as exc:
                    message = f'{type(exc).__name__}: {exc}'
                    now = time.monotonic()
                    if last_errors.get(pid) != message or now - last_report.get(pid, 0) >= 10:
                        emit({'type': 'capture_error', 'pid': pid, 'unix_ns': time.time_ns(), 'error': message})
                        last_errors[pid], last_report[pid] = message, now
                    try:
                        died = process_identity(pid) != identities[pid]
                    except OSError:
                        died = True
                    if died:
                        stopped = f'process {pid} exited or changed identity'
                        break
            if len(clients) == len(args.pids):
                cycle_end = time.time_ns()
                pair = [position(client) for client in clients[:2]]
                clock_delta = clients[1]['server_world_time_seconds'] - clients[0]['server_world_time_seconds']
                clocks.append(clock_delta)
                comparison = {'clock_estimate_delta_seconds_client2_minus_client1': clock_delta}
                if all(pair):
                    distance = math.dist(pair[0][1], pair[1][1])
                    distances.append(distance)
                    comparison.update({'aircraft_actors': [pair[0][0], pair[1][0]], 'aircraft_distance': distance})
                emit({'type': 'sample', 'start_unix_ns': cycle_start, 'end_unix_ns': cycle_end,
                      'clients': clients, 'first_pair': comparison})
                captured += 1
            if died:
                break
            next_sample += args.interval
            # Do not issue bursts when a capture takes longer than the interval.
            if next_sample < time.monotonic():
                next_sample = time.monotonic() + args.interval
        end_wall = time.time_ns()
        emit({'type': 'completion', 'start_unix_ns': start_wall, 'end_unix_ns': end_wall,
              'reason': stopped, 'samples': captured, 'aircraft_pair_valid_samples': len(distances),
              'aircraft_distance_range': [min(distances), max(distances)] if distances else None,
              'clock_estimate_delta_range_seconds': [min(clocks), max(clocks)] if clocks else None,
              'limitations': LIMITATIONS})
    print(f'{stopped}; samples={captured}; aircraft pair valid={len(distances)}; '
          f'distance range={([min(distances), max(distances)] if distances else None)}; '
          f'clock estimate delta seconds range={([min(clocks), max(clocks)] if clocks else None)} '
          '(sequential measurements)')
    return 1 if stopped != 'duration elapsed' else 0


if __name__ == '__main__':
    raise SystemExit(main())
