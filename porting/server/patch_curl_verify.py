#!/usr/bin/env python3
"""Clear FCurlHttpManager::CurlRequestOptions.bVerifyPeer in the running original client.

The shipping build compiles out the n.VerifyPeer config override and pins certificates, so
config cannot disable verification. The option struct lives in .data; this finds it by its
default signature (bVerifyPeer=1, MaxHostConnections=16, empty LocalHostAddr) and writes 0 to
the first byte. Memory of the running process only; no file is modified. Needs root (ptrace).
Usage: patch_curl_verify.py timeout_seconds client_log_path
"""
import re
import subprocess
import sys
import time

BASE = 0x140000000
DATA_RVA, DATA_SIZE = 0x7812000, 0x8bc000  # .data section of the 12.60 shipping exe
EXE = "FortniteClient-Win64-Shipping.exe"
LOG_MARKER = b"CurlRequestOptions"  # logged once the options struct is initialised
# bVerifyPeer, flags, pad, MaxHostConnections=16, LocalHostAddr FString (null data, Num 0)
SIGNATURE = re.compile(rb"\x01[\x00\x01][\x00\x01]\x00\x10\x00\x00\x00\x00{8}\x00{4}", re.S)


LOG_PATH = sys.argv[2] if len(sys.argv) > 2 else ""


def find_pid():
    out = subprocess.run(["pgrep", "-f", f"^{EXE}"], capture_output=True, text=True).stdout.split()
    return int(out[0]) if out else None


def log_ready(log_path):
    try:
        with open(log_path, "rb") as stream:
            return LOG_MARKER in stream.read()
    except OSError:
        return False


def main():
    deadline = time.time() + float(sys.argv[1] if len(sys.argv) > 1 else 90)
    pid = None
    while time.time() < deadline:
        pid = pid or find_pid()
        if pid and log_ready(LOG_PATH):
            try:
                with open(f"/proc/{pid}/mem", "r+b", buffering=0) as mem:
                    mem.seek(BASE + DATA_RVA)
                    data = mem.read(DATA_SIZE)
                    hits = [m.start() for m in SIGNATURE.finditer(data) if m.start() % 8 == 0]
                    # Only patch after Init set MaxHostConnections; the first match is the options struct.
                    print(f"candidates {[hex(BASE + DATA_RVA + h) for h in hits[:10]]}", flush=True)
                    if hits:
                        for hit in hits:
                            mem.seek(BASE + DATA_RVA + hit)
                            mem.write(b"\x00")
                        print(f"{time.strftime('%H:%M:%S', time.gmtime())} cleared bVerifyPeer at {len(hits)} candidate(s)", flush=True)
                        return 0
            except (OSError, ValueError):
                pass
        time.sleep(0.05)
    print("signature not found", flush=True)
    return 1


if __name__ == "__main__":
    sys.exit(main())
