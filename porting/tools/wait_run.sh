#!/usr/bin/env bash
# Wait for a vanilla_play_run.sh run: exit 0 when REGEX appears in the run's server.log or client log,
# exit 3 when the run has ended first (client crash or timeout), exit 4 on TIMEOUT seconds.
# Usage: tools/wait_run.sh RUN_DIR REGEX [TIMEOUT]
run=$1 pattern=$2 limit=${3:-900}
porting=$(cd -- "$(dirname -- "$0")/.." && pwd)
client_logs=("${FN_WINEPREFIX:-/tmp/fortnite-dxvk-experiment/prefix}"/drive_c/users/*/AppData/Local/FortniteGame/Saved/Logs/FortniteGame.log
             "$porting"/generated/wine-prefix/drive_c/users/*/AppData/Local/FortniteGame/Saved/Logs/FortniteGame.log)
deadline=$((SECONDS + limit))
while ((SECONDS < deadline)); do
    if grep -aqE "$pattern" "$run/server.log" 2>/dev/null; then echo "matched in server.log"; exit 0; fi
    for log in "${client_logs[@]}"; do
        [[ -f $log && $log -nt $run/start.marker ]] && grep -aqE "$pattern" "$log" && { echo "matched in $log"; exit 0; }
    done
    # vanilla_play_run.sh writes this line only after the client runner has exited.
    if grep -aq 'Original client runner exit status' "$run/runner.out" 2>/dev/null; then
        echo "run ended: $(grep -a 'Original client runner exit status' "$run/runner.out" | tail -1)"; exit 3
    fi
    sleep 3
done
echo "timeout after $limit s"; exit 4
