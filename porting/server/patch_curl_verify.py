#!/usr/bin/env python3
"""Clear FCurlHttpManager::CurlRequestOptions.bVerifyPeer in the running original client.

The shipping build compiles out the n.VerifyPeer config override and pins certificates, so
config cannot disable verification. The original sslctx_function reads bVerifyPeer at
0x147c69478. Validate its initialized option signature and clear only that byte;
the same signature also occurs elsewhere and does not establish another option address.
Memory of the running process only; no file is modified. Needs root (ptrace).
Usage: patch_curl_verify.py timeout_seconds client_log_path
"""
import re
import os
import sys
import time

from client_process import shipping_client_pids

OPTIONS_ADDRESS = 0x147c69478  # original 0x140c593b3 reads this bVerifyPeer byte
LOG_MARKER = b"CurlRequestOptions"  # logged once the options struct is initialised
# bVerifyPeer, flags, pad, MaxHostConnections=16, LocalHostAddr FString (null data, Num 0)
SIGNATURE = re.compile(rb"\x01[\x00\x01][\x00\x01]\x00\x10\x00\x00\x00\x00{8}\x00{4}", re.S)


LOG_PATH = sys.argv[2] if len(sys.argv) > 2 else ""


def log_ready(log_path):
    try:
        with open(log_path, "rb") as stream:
            return LOG_MARKER in stream.read()
    except OSError:
        return False


def main():
    if os.geteuid() != 0 or os.readlink('/proc/self/ns/net') == os.readlink('/proc/1/ns/net'):
        print('diagnostic patch requires root in a private network namespace', file=sys.stderr)
        return 1
    deadline = time.time() + float(sys.argv[1] if len(sys.argv) > 1 else 90)
    pid = None
    while time.time() < deadline:
        pid = pid or next(iter(shipping_client_pids()), None)
        if pid and log_ready(LOG_PATH):
            try:
                with open(f"/proc/{pid}/mem", "r+b", buffering=0) as mem:
                    mem.seek(OPTIONS_ADDRESS)
                    data = mem.read(24)
                    # Wait until Init sets MaxHostConnections and LocalHostAddr.
                    if SIGNATURE.match(data):
                        mem.seek(OPTIONS_ADDRESS)
                        mem.write(b"\x00")
                        mem.seek(OPTIONS_ADDRESS)
                        if mem.read(1) != b"\x00":
                            raise RuntimeError('diagnostic patch verification failed')
                        print(f"{time.strftime('%H:%M:%S', time.gmtime())} cleared bVerifyPeer at observed option address", flush=True)
                        return 0
            except (OSError, ValueError):
                pass
        time.sleep(0.05)
    print("signature not found", flush=True)
    return 1


if __name__ == "__main__":
    sys.exit(main())
