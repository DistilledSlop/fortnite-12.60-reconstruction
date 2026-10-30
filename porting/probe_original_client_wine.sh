#!/usr/bin/env bash
# Bounded startup probe of the supplied main Windows client. No launcher wrapper.
set -euo pipefail

porting_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
install_dir=$(cd -- "$porting_dir/.." && pwd)
client="$install_dir/Fortnite/FortniteGame/Binaries/Win64/FortniteClient-Win64-Shipping.exe"
generated="$porting_dir/generated"
prefix="$generated/wine-prefix"
log="$generated/original-client-wine-probe.log"
seconds=${1:-180}

if [[ ! "$seconds" =~ ^[0-9]+$ ]] || (( seconds < 1 || seconds > 600 )); then
    printf 'Duration must be 1..600 seconds.\n' >&2
    exit 2
fi
if [[ ! -f "$client" ]]; then
    printf 'Main Windows client is missing: %s\n' "$client" >&2
    exit 2
fi
mkdir -p "$generated"
ulimit -c 0 || true

set +e
WINEPREFIX="$prefix" WINEDEBUG=-all xvfb-run -a -s '-screen 0 1280x720x24' \
    timeout "${seconds}s" wine "$client" -NoEpicPortal -nullrhi -nosound \
    -NoSplash -unattended -log > "$log" 2>&1
status=$?
set -e

python3 - "$log" "$status" <<'PY'
from collections import Counter
import re
import sys

path, status = sys.argv[1:]
data = open(path, encoding="utf-8", errors="ignore").read()
counts = Counter(re.findall(r"\bLog[A-Za-z0-9_]+:", data))
service = {name: count for name, count in counts.items()
           if any(word in name for word in ("Http", "Mcp", "Online", "Net", "Login", "Match", "UAC"))}
print(f"Original client probe: exit={status}, log={path}, lines={data.count(chr(10))}")
print(f"Service log categories: {service}")
PY

# 124 means the bounded probe reached its time limit while the client ran.
if [[ $status -eq 124 ]]; then
    exit 0
fi
exit "$status"
