#!/usr/bin/env bash
# Build the Linux dedicated server and, with --stage, copy it into the server stage used by play runs.
# Log: porting/generated/builds/<YYYYMMDD-HHMMSS>-server.log. Exit status is non-zero on a failed build
# (a failed build leaves the old binary in place, so never stage without checking).
# Usage: tools/build_server.sh [--stage]
set -uo pipefail
porting=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
engine=${UE_ROOT:-/opt/fortnite/UnrealEngine}
log_dir="$porting/generated/builds"
mkdir -p "$log_dir"
log="$log_dir/$(date +%Y%m%d-%H%M%S)-server.log"
echo "Build log: $log"
"$engine/Engine/Build/BatchFiles/Linux/Build.sh" FortniteLinuxServer Linux Development \
    -project="$porting/native/FortniteLinux.uproject" -NoHotReload > "$log" 2>&1
status=$?
errors=$(grep -c ' error:\|^error:' "$log")
if ((status != 0 || errors > 0)); then
    echo "Build FAILED (exit $status, $errors error lines):"
    grep -m 10 ' error:\|^error:' "$log"
    exit 1
fi
echo "Build OK ($(tail -1 "$log"))"
if [[ ${1:-} == --stage ]]; then
    cp "$porting/native/Binaries/Linux/FortniteLinuxServer" \
       "$porting/generated/linux-stage/LinuxNoEditor/FortniteLinux/Binaries/Linux/FortniteLinuxServer"
    echo "Staged into generated/linux-stage."
fi
