#!/usr/bin/env python3
"""Opt-in 12.60 WSS certificate-validation bypass for isolated diagnostics.

This changes one byte in the running client only; it never modifies the EXE.
It forces the existing certificate-validation-disabled branch and therefore
bypasses WSS certificate validation. Secure trust remains unresolved (G11).
Run as root only in a private network namespace used for the diagnostic probe.
"""
import argparse
import os
import sys
import time

from client_process import shipping_client_pids

# Both the conditional branch and its existing success target must match the
# observed 12.60 runtime code before changing the single branch opcode.
BRANCH_CONTEXT_ADDRESS = 0x144866E28
TARGET_CONTEXT_ADDRESS = 0x144866E8B
BRANCH_ADDRESS = 0x144866E35
# Populated from the verified runtime instruction bytes.
BRANCH_CONTEXT = bytes.fromhex('e8 83 a3 40 fe 45 33 f6 44 38 74 24 60 75 54 44 8b e6')
TARGET_CONTEXT = bytes.fromhex('33 d2 48 8b cf e8 0b 95 b5 00 41 83 fc 01 41 0f 95 c6 41 8b c6 e9 ef fb ff ff')


def patch_memory(mem):
    """Patch a matching process memory stream; return applied/already/mismatch."""
    mem.seek(BRANCH_CONTEXT_ADDRESS)
    branch = mem.read(len(BRANCH_CONTEXT))
    mem.seek(TARGET_CONTEXT_ADDRESS)
    target = mem.read(len(TARGET_CONTEXT))
    offset = BRANCH_ADDRESS - BRANCH_CONTEXT_ADDRESS
    patched = BRANCH_CONTEXT[:offset] + b'\xeb' + BRANCH_CONTEXT[offset + 1:]
    if target != TARGET_CONTEXT or branch not in (BRANCH_CONTEXT, patched):
        return 'mismatch'
    if branch == patched:
        return 'already'
    mem.seek(BRANCH_ADDRESS)
    if mem.write(b'\xeb') != 1:
        raise RuntimeError('diagnostic patch write failed')
    mem.seek(BRANCH_ADDRESS)
    if mem.read(1) != b'\xeb':
        raise RuntimeError('diagnostic patch verification failed')
    return 'applied'


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('timeout', type=float, help='seconds to poll for the client')
    args = parser.parse_args(argv)
    if not 0 < args.timeout < float('inf'):
        parser.error('timeout must be finite and positive')
    if os.geteuid() != 0:
        print('diagnostic patch requires root', file=sys.stderr)
        return 1
    try:
        namespace = os.readlink('/proc/self/ns/net')
        if namespace == os.readlink('/proc/1/ns/net'):
            print('diagnostic patch requires a private network namespace', file=sys.stderr)
            return 1
    except OSError:
        print('cannot verify network namespace', file=sys.stderr)
        return 1
    deadline = time.monotonic() + args.timeout
    while time.monotonic() < deadline:
        for pid in shipping_client_pids():
            try:
                if os.readlink(f'/proc/{pid}/ns/net') != namespace:
                    continue
                with open(f'/proc/{pid}/mem', 'r+b', buffering=0) as mem:
                    result = patch_memory(mem)
                if result != 'mismatch':
                    print(f'diagnostic WSS patch {result} pid {pid} address {BRANCH_ADDRESS:#x}', flush=True)
                    return 0
            except (OSError, ValueError):
                continue
        time.sleep(0.05)
    print('expected diagnostic signatures not found before timeout', file=sys.stderr)
    return 1


if __name__ == '__main__':
    sys.exit(main())
