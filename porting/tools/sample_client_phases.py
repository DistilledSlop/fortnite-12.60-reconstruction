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

from probe_athena_phase import snapshot
from probe_equipment_pose import component, req

LIMITATIONS = [
    'Client and component reads are sequential, not an atomic game tick.',
    'Clock and distance differences describe sequential measurements, not causality or acceptance.',
    'Aircraft root components are read after the validated phase snapshot; actor lifetime may change between reads.',
]


def process_identity(pid):
    # The command name in stat may contain spaces and parentheses.
    stat = Path(f'/proc/{pid}/stat').read_text()
    fields = stat[stat.rfind(')') + 2:].split()
    if fields[0] in ('Z', 'X'):
        raise ProcessLookupError(f'process {pid} is no longer running')
    return fields[19]  # starttime, field 22


def capture(pid):
    result = snapshot(pid, None)
    mem = os.open(f'/proc/{pid}/mem', os.O_RDONLY)
    try:
        for aircraft in result['aircraft']:
            if aircraft is None:
                continue
            actor = int(aircraft['object']['address'], 16)
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
                    client = capture(pid)
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
