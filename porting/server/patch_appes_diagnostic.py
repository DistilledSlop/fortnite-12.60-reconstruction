#!/usr/bin/env python3
"""Suppress a 12.60 unsafe-environment handler and timed exit in Wine.

Diagnostic only: this does not establish that the underlying launcher or
entitlement checks succeeded. Callback mode suppresses all callback uses;
timer mode suppresses the observed registration call site, whose full scope
is unknown. The executable on disk is not modified.
"""
import os
import sys
import time

from client_process import shipping_client_pids

ADDRESS = 0x140000000 + 0x2121984
EXPECTED = bytes.fromhex('0f 84 7b 03 00 00')
# Take the handler's existing early-out, which performs argument cleanup.
REPLACEMENT = bytes.fromhex('e9 7c 03 00 00 90')
# A timer delegate targeted this callback and reached RequestExit(false) after
# 20 seconds in the 12.60 Wine probe. Returning here suppresses every invocation
# of this callback, not only the observed timer.
EXIT_CALLBACK = 0x142120970
EXIT_EXPECTED = bytes.fromhex('e9 91 bb 83 00 90')
EXIT_REPLACEMENT = bytes.fromhex('c3 90 90 90 90 90')
# The observed 20-second timer registration call. This is narrower than
# replacing the callback, but other uses of this call site remain unknown.
TIMER_CALL = 0x14211ED87
TIMER_EXPECTED = bytes.fromhex('e8 44 8b 50 02')
TIMER_REPLACEMENT = bytes.fromhex('90 90 90 90 90')


def main():
    deadline = time.monotonic() + float(sys.argv[1])
    log_path = sys.argv[2]
    mode = sys.argv[3] if len(sys.argv) > 3 else 'callback'
    if mode not in ('callback', 'timer'):
        raise ValueError('mode must be callback or timer')
    exit_address, exit_expected, exit_replacement = (
        (EXIT_CALLBACK, EXIT_EXPECTED, EXIT_REPLACEMENT) if mode == 'callback'
        else (TIMER_CALL, TIMER_EXPECTED, TIMER_REPLACEMENT))
    while time.monotonic() < deadline:
        try:
            with open(log_path, 'rb') as log:
                if b'LogLoad: LoadMap: /Game/Maps/Frontend' not in log.read():
                    time.sleep(0.05)
                    continue
        except OSError:
            time.sleep(0.05)
            continue
        for pid in shipping_client_pids():
            try:
                if os.readlink(f'/proc/{int(pid)}/ns/net') != os.readlink('/proc/self/ns/net'):
                    continue
                with open(f'/proc/{int(pid)}/mem', 'r+b', buffering=0) as mem:
                    mem.seek(ADDRESS)
                    actual = mem.read(len(EXPECTED))
                    mem.seek(exit_address)
                    callback = mem.read(len(exit_expected))
                    if actual == EXPECTED and callback == exit_expected:
                        mem.seek(exit_address)
                        mem.write(exit_replacement)
                        mem.seek(exit_address)
                        if mem.read(len(exit_replacement)) != exit_replacement:
                            raise RuntimeError('exit patch verification failed')
                        mem.seek(ADDRESS)
                        mem.write(REPLACEMENT)
                        mem.seek(ADDRESS)
                        if mem.read(len(REPLACEMENT)) != REPLACEMENT:
                            raise RuntimeError('patch verification failed')
                        print(f'diagnostic handler and {mode} exit patch applied to pid {pid}', flush=True)
                        return 0
            except (OSError, ValueError):
                pass
        time.sleep(0.05)
    print('expected handler and callback signatures not found before timeout', flush=True)
    return 1


if __name__ == '__main__':
    sys.exit(main())
