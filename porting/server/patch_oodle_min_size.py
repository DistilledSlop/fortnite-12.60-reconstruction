#!/usr/bin/env python3
"""Keep the original client from compressing packets (net.OodleMinSizeForCompression).

The 12.60 client's packet handler chain starts with an Oodle component that puts a "compressed
packet" bit in front of every packet and Oodle-compresses packets of 19 bytes or more. Our server
has no Oodle SDK, so the client must never compress. The cvar is set by shipped config at startup
and the loose-ini overrides do not win (the same reason the TLS check needs a memory patch), so this
writes the value in the running process.

Facts (binary, `FUN_1402132f0`): the static initializer registers the cvar and stores the
IConsoleVariable* at 0x1480eb2e8 and the TConsoleVariableData<int32>* (two shadowed values, game
and render thread) at 0x1480eb2f0. This script reads that pointer and writes the target value to
both shadowed ints, then keeps re-applying until the timeout so a later config pass cannot undo it.
Memory of the running process only; no file is modified. Needs root (ptrace).
Usage: patch_oodle_min_size.py timeout_seconds [target_value]
"""
import struct
import sys
import time

from client_process import shipping_client_pids

PTR_ADDR = 0x1480EB2F0  # TConsoleVariableData<int32>* for net.OodleMinSizeForCompression


def main():
    deadline = time.time() + float(sys.argv[1] if len(sys.argv) > 1 else 600)
    target = int(sys.argv[2]) if len(sys.argv) > 2 else 100000
    pid = None
    last = None
    applied = 0
    while time.time() < deadline:
        pid = pid or next(iter(shipping_client_pids()), None)
        if pid:
            try:
                with open(f"/proc/{pid}/mem", "r+b", buffering=0) as mem:
                    mem.seek(PTR_ADDR)
                    (ptr,) = struct.unpack("<Q", mem.read(8))
                    if 0x10000 < ptr < 0x7FFFFFFFFFFF:
                        mem.seek(ptr)
                        values = struct.unpack("<ii", mem.read(8))
                        if values != last:
                            print(f"{time.strftime('%H:%M:%S', time.gmtime())} cvar data at {ptr:#x}: {values}", flush=True)
                            last = values
                        # Only touch plausible values (default 0, shipped 19, or our own target).
                        if values[0] != target and 0 <= values[0] < 1000:
                            mem.seek(ptr)
                            mem.write(struct.pack("<ii", target, target))
                            applied += 1
                            print(f"{time.strftime('%H:%M:%S', time.gmtime())} set net.OodleMinSizeForCompression to {target} (write {applied})", flush=True)
            except (OSError, ValueError, struct.error):
                pass
        time.sleep(0.25)
    print(f"done; writes applied: {applied}", flush=True)
    return 0 if applied else 1


if __name__ == "__main__":
    sys.exit(main())
