#!/usr/bin/env python3
"""Suppress the 12.60 unsafe-environment handler in one running Wine process.

Diagnostic only: this does not establish that the underlying launcher or
entitlement checks succeeded. The original executable on disk is not modified.
"""
import os
import subprocess
import sys
import time

ADDRESS = 0x140000000 + 0x2121984
EXPECTED = bytes.fromhex('0f 84 7b 03 00 00')
# Take the handler's existing early-out, which performs argument cleanup.
REPLACEMENT = bytes.fromhex('e9 7c 03 00 00 90')


def main():
    deadline = time.monotonic() + float(sys.argv[1])
    log_path = sys.argv[2]
    while time.monotonic() < deadline:
        try:
            with open(log_path, 'rb') as log:
                if b'LoginWaitingRoom' not in log.read():
                    time.sleep(0.05)
                    continue
        except OSError:
            time.sleep(0.05)
            continue
        result = subprocess.run(
            ['pgrep', '-f', '^FortniteClient-Win64-Shipping.exe'],
            capture_output=True, text=True, check=False)
        for pid in result.stdout.split():
            try:
                if os.readlink(f'/proc/{int(pid)}/ns/net') != os.readlink('/proc/self/ns/net'):
                    continue
                with open(f'/proc/{int(pid)}/mem', 'r+b', buffering=0) as mem:
                    mem.seek(ADDRESS)
                    actual = mem.read(len(EXPECTED))
                    if actual == EXPECTED:
                        mem.seek(ADDRESS)
                        mem.write(REPLACEMENT)
                        mem.seek(ADDRESS)
                        if mem.read(len(REPLACEMENT)) != REPLACEMENT:
                            raise RuntimeError('patch verification failed')
                        print(f'diagnostic handler early-out applied to pid {pid}', flush=True)
                        return 0
            except (OSError, ValueError):
                pass
        time.sleep(0.05)
    print('decrypted handler signature not found before timeout', flush=True)
    return 1


if __name__ == '__main__':
    sys.exit(main())
