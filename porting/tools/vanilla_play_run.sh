#!/usr/bin/env bash
set -uo pipefail
porting=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
seconds=${1:-420}
[[ $seconds =~ ^[1-9][0-9]*$ ]] || { echo 'Usage: vanilla_play_run.sh [positive seconds]' >&2; exit 2; }
registered_mode=${FN_REGISTERED_MATCHMAKING:-0}
[[ $registered_mode == 0 || $registered_mode == 1 ]] || { printf 'Invalid FN_REGISTERED_MATCHMAKING=%s (expected 0 or 1).\n' "$registered_mode" >&2; exit 2; }
network_trace=${FN_NETWORK_TRACE:-0}
[[ $network_trace == 0 || $network_trace == 1 ]] || { echo 'FN_NETWORK_TRACE must be 0 or 1.' >&2; exit 2; }
if [[ $registered_mode == 1 ]]; then
    export FN_REGISTERED_MATCHMAKING=1
    export FORTNITE_GAMESERVER_ADDRESS="${FORTNITE_GAMESERVER_ADDRESS:-127.0.0.1}"
    export FORTNITE_GAMESERVER_PLAYLIST="${FORTNITE_GAMESERVER_PLAYLIST:-Playlist_DefaultSolo}"
    export FORTNITE_GAMESERVER_BUILD="${FORTNITE_GAMESERVER_BUILD:-12896485}"
    export FORTNITE_GAMESERVER_PORT="${FORTNITE_GAMESERVER_PORT:-7777}"
    export FORTNITE_GAMESERVER_BEACON_PORT="${FORTNITE_GAMESERVER_BEACON_PORT:-7778}"
    export FORTNITE_GAMESERVER_CAPACITY="${FORTNITE_GAMESERVER_CAPACITY:-100}"
    "$porting/run_original_client.sh" --validate-registered || exit 2
fi
# Standard run folder: generated/runs/<YYYYMMDD>-<label>-<n>/ (label from FN_RUN_LABEL, default play-run).
base="$porting/generated/runs/$(date +%Y%m%d)-${FN_RUN_LABEL:-play-run}"
mkdir -p "$(dirname "$base")"
n=1
while ! mkdir "$base-$n" 2>/dev/null; do
    [[ -e "$base-$n" ]] || { echo 'Cannot create output directory' >&2; exit 1; }
    ((n++))
done
out="$base-$n"
mkdir -p "$out/shots"
if [[ $network_trace == 1 ]]; then
    export FN_NETWORK_TRACE_PATH="$out/network-rpc.jsonl"
else
    unset FN_NETWORK_TRACE_PATH
fi
export FN_SHOTS_DIR="$out/shots"
# Override for targeted diagnostics, e.g. LogNetTraffic Verbose.
export FN_SERVER_LOG_CMDS="${FN_SERVER_LOG_CMDS:-LogHandshake Log, LogNet Log, LogNetTraffic Warning, PacketHandlerLog Log}"
# This script's own output is kept in the run folder too (callers need not redirect it anywhere else).
exec > >(tee -a "$out/play_run.out") 2>&1
python3 - "$out/meta.json" "$seconds" <<'PYEOF'
import json, os, subprocess, sys, time
rev = subprocess.run(['git', 'rev-parse', '--short', 'HEAD'], capture_output=True, text=True).stdout.strip()
keys = ['FN_RUN_LABEL', 'FN_RENDERER', 'FN_SERVER_MAP', 'FN_SERVER_URL_OPTS', 'FN_SERVER_ARGS', 'FN_SERVER_LOG_CMDS', 'FN_STATE_DIR', 'FN_ACTIONS', 'FN_LP_NUM_THREADS', 'FN_SECOND_CLIENT', 'FN_SECOND_USERNAME', 'FN_SECOND_RENDERER', 'FN_SECOND_ACTIONS', 'FN_SESSION_TRACE', 'FN_NETWORK_TRACE', 'FN_NETWORK_TRACE_PATH', 'FN_REGISTERED_MATCHMAKING', 'FORTNITE_GAMESERVER_SECRET_FILE', 'FORTNITE_GAMESERVER_ADDRESS', 'FORTNITE_GAMESERVER_PLAYLIST', 'FORTNITE_GAMESERVER_BUILD', 'FORTNITE_GAMESERVER_PORT', 'FORTNITE_GAMESERVER_BEACON_PORT', 'FORTNITE_GAMESERVER_CAPACITY']
json.dump({'started': time.strftime('%Y-%m-%dT%H:%M:%S%z'), 'seconds': int(sys.argv[2]), 'git_head': rev,
           'env': {k: os.environ[k] for k in keys if k in os.environ}}, open(sys.argv[1], 'w'), indent=1)
PYEOF
runner_pid= capture_pid= server_pid= second_pid=
cleanup() {
    if [[ -n $second_pid ]]; then kill -TERM "$second_pid" 2>/dev/null || true; wait "$second_pid" 2>/dev/null || true; second_pid=; fi
    if [[ -n $capture_pid ]]; then sudo -n kill -INT "$capture_pid" 2>/dev/null || true; wait "$capture_pid" 2>/dev/null || true; fi
    if [[ -n $runner_pid ]] && kill -0 "$runner_pid" 2>/dev/null; then sudo -n kill "$runner_pid" 2>/dev/null || true; wait "$runner_pid" 2>/dev/null || true; fi
    if [[ -n $server_pid ]] && kill -0 "$server_pid" 2>/dev/null; then sudo -n kill "$server_pid" 2>/dev/null || true; fi
    [[ ! -e $out/udp.pcap ]] || sudo -n chown "$(id -u):$(id -g)" "$out/udp.pcap" 2>/dev/null || true
}
trap cleanup EXIT INT TERM
printf 'Output directory: %s\nWaiting for Wine to become idle (up to 300 seconds)...\n' "$out"
for ((i=0; i<300; i++)); do
    # Only this run's prefix has to be idle; other agents' probes use their own prefixes (FN_WINEPREFIX unset = default prefix).
    if ! pgrep -f "[W]INEPREFIX=${FN_WINEPREFIX:-$porting/generated/wine-prefix} .*FortniteClient-Win64-Shipping" >/dev/null 2>&1 && WINEPREFIX="${FN_WINEPREFIX:-$porting/generated/wine-prefix}" timeout 5 /usr/lib/x86_64-linux-gnu/wine/wineserver -w 2>/dev/null; then break; fi
    sleep 1
done
if ((i == 300)); then echo 'Wine prefix still busy after 300 seconds.' >&2; exit 1; fi
# Record existing server PIDs so an unrelated server is never captured or stopped.
old_servers=" $(pgrep -x FortniteLinuxSe | tr '\n' ' ') "
server="$porting/generated/linux-stage/LinuxNoEditor/FortniteLinux/Binaries/Linux/FortniteLinuxServer"
if [[ $registered_mode == 1 ]]; then
    unset FORTNITE_NET_ENCRYPTION_KEY FORTNITE_NET_ENCRYPTION_KEY_FILE
else
    if [[ ! -f "$porting/generated/g3/netkey" ]]; then
        # Local test key shared by the server and the backend; generated, ignored by Git, never printed.
        mkdir -p "$porting/generated/g3" && python3 -c 'import os,sys; open(sys.argv[1],"w").write(os.urandom(32).hex())' "$porting/generated/g3/netkey" && chmod 600 "$porting/generated/g3/netkey"
    fi
    export FORTNITE_NET_ENCRYPTION_KEY_FILE="$porting/generated/g3/netkey"
fi
export FN_PATCH_APPES_DIAGNOSTIC=1 FN_WINEDEBUG=-all
unset FN_WINEDLLOVERRIDES FN_VK_ICD_FILENAMES
# FN_RENDERER=dxvk: render through DXVK on the software Vulkan driver (lavapipe) so the 3D world is drawn. Needs a prefix with
# DXVK installed (FN_DXVK_PREFIX, default /tmp/fortnite-dxvk-experiment/prefix; the default prefix has Wine's own d3d11/dxgi).
if [[ ${FN_RENDERER:-} == dxvk ]]; then
    export FN_WINEPREFIX="${FN_DXVK_PREFIX:-/tmp/fortnite-dxvk-experiment/prefix}"
    export FN_WINEDLLOVERRIDES='d3d11,dxgi=n'
    export FN_VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/lvp_icd.json
    export FN_LP_NUM_THREADS="${FN_LP_NUM_THREADS:-8}"
    [[ -d $FN_WINEPREFIX ]] || { echo "DXVK prefix $FN_WINEPREFIX not found" >&2; exit 1; }
fi
export FN_ACTIONS="${FN_ACTIONS:-125:click,640,592 235:click,1144,473 300:shot 400:hold,w,6 410:shot 430:hold,d,6 440:shot 560:shot}"
# Extra server URL options (e.g. '?RequiredPlayers=1') and switches (e.g. '-FortniteWarmupSeconds=20').
server_url="${FN_SERVER_MAP:-/Game/Maps/FortniteEmptyDedicated}${FN_SERVER_URL_OPTS:-}"
read -r -a server_extra <<< "${FN_SERVER_ARGS:-}"
if [[ $network_trace == 1 ]]; then
    server_extra+=("-FortniteNetworkTrace=$FN_NETWORK_TRACE_PATH")
fi
server_port=7777
if [[ $registered_mode == 1 ]]; then
    server_port="$FORTNITE_GAMESERVER_PORT"
    if [[ " ${server_extra[*]+"${server_extra[*]}"} " != *" -FortniteRegisteredMatchmaking "* ]]; then
        server_extra=(-FortniteRegisteredMatchmaking ${server_extra[@]+"${server_extra[@]}"})
    fi
fi
printf -v FN_SERVER_CMD 'exec %q ' "$server" "$server_url" -FortniteOriginalClient -nullrhi -nosound -unattended "-port=$server_port" -log "-LogCmds=$FN_SERVER_LOG_CMDS" "${server_extra[@]}"
printf -v server_redirect '> %q 2>&1' "$out/server.log"
FN_SERVER_CMD+="$server_redirect"
export FN_SERVER_CMD
export FN_ACTION_LOG="$out/server.log"
touch "$out/start.marker"
"$porting/run_original_client.sh" "$seconds" "${@:2}" > "$out/runner.out" 2>&1 & runner_pid=$!
# Start after the first frontend appears, while its private namespaces and backend are alive.
# Prepare the second prefix before the run: second_client.sh --prepare.
if [[ ${FN_SECOND_CLIENT:-0} == 1 ]]; then
    (
        first_log="${FN_WINEPREFIX:-$porting/generated/wine-prefix}/drive_c/users/$USER/AppData/Local/FortniteGame/Saved/Logs/FortniteGame.log"
        while kill -0 "$runner_pid" 2>/dev/null; do
            if [[ $first_log -nt $out/start.marker ]] && grep -aq 'ShouldShowBeginningOfSeasonDialog' "$first_log"; then
                exec "$porting/tools/second_client.sh" "$out" \
                    --username "${FN_SECOND_USERNAME:-LocalPlayer2}" \
                    --renderer "${FN_SECOND_RENDERER:-${FN_RENDERER:-builtin}}" \
                    --actions "${FN_SECOND_ACTIONS:-$FN_ACTIONS}"
            fi
            sleep 1
        done
        echo 'First client ended before second client could start.' >&2
        exit 1
    ) > "$out/second-client-start.out" 2>&1 & second_pid=$!
fi
is_runner_child() {
    local p=$1
    while [[ $p =~ ^[0-9]+$ && $p -gt 1 ]]; do
        [[ $p == "$runner_pid" ]] && return 0
        p=$(ps -o ppid= -p "$p" 2>/dev/null | tr -d ' ')
    done
    return 1
}
for ((i=0; i<40; i++)); do
    for pid in $(pgrep -x FortniteLinuxSe || true); do
        if [[ $old_servers != *" $pid "* ]] && is_runner_child "$pid"; then server_pid=$pid; break; fi
    done
    [[ -z $server_pid ]] || break
    kill -0 "$runner_pid" 2>/dev/null || break
    sleep 1
done
if [[ -n $server_pid ]]; then
    sudo -n nsenter -t "$server_pid" -n tcpdump -i lo -n -U -w "$out/udp.pcap" udp port "$server_port" > "$out/tcpdump.log" 2>&1 & capture_pid=$!
else
    echo 'Server did not appear within 40 seconds; see runner.out.' >&2
fi
wait "$runner_pid"; status=$?; runner_pid=
if [[ -n $second_pid ]]; then
    # The helper normally stops itself when the first client exits; also stop a
    # monitor still waiting for the frontend. Its controller forwards the signal.
    if kill -0 "$second_pid" 2>/dev/null; then
        kill -TERM "$second_pid" 2>/dev/null || true
        wait "$second_pid" 2>/dev/null || true
    elif ! wait "$second_pid"; then
        echo 'Second client failed; see client2/runner.out and second-client-start.out' >&2
        status=1
    fi
    second_pid=
fi
if [[ -n $capture_pid ]]; then sudo -n kill -INT "$capture_pid" 2>/dev/null || true; wait "$capture_pid" 2>/dev/null || true; capture_pid=; fi
[[ ! -e $out/udp.pcap ]] || sudo -n chown "$(id -u):$(id -g)" "$out/udp.pcap"
client="${FN_WINEPREFIX:-$porting/generated/wine-prefix}/drive_c/users/${USER}/AppData/Local/FortniteGame/Saved/Logs/FortniteGame.log"
[[ ! -f $client ]] || cp "$client" "$out/client.log"
printf '\nOutput directory: %s\nRunner exit status: %s\n' "$out" "$status"
# The runner translates its normal timeout into exit 0; preserve its reported status too.
grep -a 'Original client runner exit status:' "$out/runner.out" | tail -1 || true
printf 'Screenshots:'; for shot in "$out"/shots/t*.png; do [[ ! -f $shot ]] || printf ' %s' "${shot##*/}"; done; printf '\n'
for pattern in 'Server accepting post-challenge connection' 'SendRestartHandshakeRequest' 'Malformed_Packet' 'NMT_' 'Welcomed' 'UNetConnection::Close'; do
    count=$(grep -ac "$pattern" "$out/server.log" 2>/dev/null || true); printf '%s: %s\n' "$pattern" "${count:-0}"
done
printf 'Server excerpt (first 12):\n'
grep -aE 'LogHandshake|accepting post-challenge|Malformed|NMT_|Welcome|SetClientLoginState' "$out/server.log" 2>/dev/null | head -12 || true
if [[ -s $out/udp.pcap ]]; then
    count=$(tcpdump -n -r "$out/udp.pcap" 2>/dev/null | wc -l); printf 'PCAP packets: %s\n' "$count"
    decoder="$porting/tools/pcap_dump.py"
    if [[ -f $decoder ]]; then python3 "$decoder" "$out/udp.pcap" 8 | head -9; else echo 'Packet decoder unavailable.'; fi
else echo 'PCAP packets: unavailable'; fi
printf 'Client LogNet/LogHandshake (last 15):\n'
grep -aE 'LogNet|LogHandshake' "$out/client.log" 2>/dev/null | grep -avE 'VeryVerbose|TickDispatch|TickFlush' | tail -15 || true
[[ -n $server_pid && -s $out/udp.pcap ]] || exit 1
exit "$status"
