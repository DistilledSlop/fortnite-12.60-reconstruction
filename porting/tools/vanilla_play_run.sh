#!/usr/bin/env bash
set -uo pipefail
porting=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
seconds=${1:-420}
[[ $seconds =~ ^[1-9][0-9]*$ ]] || { echo 'Usage: vanilla_play_run.sh [positive seconds]' >&2; exit 2; }
base="$porting/generated/wine-investigation/$(date +%Y%m%d)-play-run"
mkdir -p "$(dirname "$base")"
n=1
while ! mkdir "$base-$n" 2>/dev/null; do
    [[ -e "$base-$n" ]] || { echo 'Cannot create output directory' >&2; exit 1; }
    ((n++))
done
out="$base-$n"
runner_pid= capture_pid= server_pid=
cleanup() {
    if [[ -n $capture_pid ]]; then sudo -n kill -INT "$capture_pid" 2>/dev/null || true; wait "$capture_pid" 2>/dev/null || true; fi
    if [[ -n $runner_pid ]] && kill -0 "$runner_pid" 2>/dev/null; then sudo -n kill "$runner_pid" 2>/dev/null || true; wait "$runner_pid" 2>/dev/null || true; fi
    if [[ -n $server_pid ]] && kill -0 "$server_pid" 2>/dev/null; then sudo -n kill "$server_pid" 2>/dev/null || true; fi
    [[ ! -e $out/udp.pcap ]] || sudo -n chown "$(id -u):$(id -g)" "$out/udp.pcap" 2>/dev/null || true
}
trap cleanup EXIT INT TERM
printf 'Output directory: %s\nWaiting for Wine to become idle (up to 300 seconds)...\n' "$out"
for ((i=0; i<300; i++)); do
    if ! pgrep -x FortniteClient-Win64-Shipping.exe >/dev/null 2>&1 && ! pgrep -x wineserver >/dev/null 2>&1 && ! ps -eo args= | grep -q '[F]ortniteClient-Win64-Shipping.exe'; then break; fi
    sleep 1
done
if ((i == 300)); then echo 'Wine prefix still busy after 300 seconds.' >&2; exit 1; fi
# Record existing server PIDs so an unrelated server is never captured or stopped.
old_servers=" $(pgrep -x FortniteLinuxSe | tr '\n' ' ') "
server="$porting/generated/linux-stage/LinuxNoEditor/FortniteLinux/Binaries/Linux/FortniteLinuxServer"
if [[ ! -f "$porting/generated/g3/netkey" ]]; then
    # Local test key shared by the server and the backend; generated, ignored by Git, never printed.
    mkdir -p "$porting/generated/g3" && python3 -c 'import os,sys; open(sys.argv[1],"w").write(os.urandom(32).hex())' "$porting/generated/g3/netkey" && chmod 600 "$porting/generated/g3/netkey"
fi
export FORTNITE_NET_ENCRYPTION_KEY_FILE="$porting/generated/g3/netkey"
export FN_PATCH_APPES_DIAGNOSTIC=1 FN_WINEDEBUG=-all
unset FN_WINEDLLOVERRIDES FN_VK_ICD_FILENAMES
export FN_ACTIONS='125:click,640,592 235:click,1144,473 300:shot 400:hold,w,6 410:shot 430:hold,d,6 440:shot 560:shot'
printf -v FN_SERVER_CMD 'exec %q /Game/Maps/FortniteEmptyDedicated -FortniteOriginalClient -nullrhi -nosound -unattended -port=7777 -log -LogCmds=%q > %q 2>&1' "$server" 'LogHandshake VeryVerbose, LogNet VeryVerbose, LogNetTraffic Verbose, PacketHandlerLog VeryVerbose' "$out/server.log"
export FN_SERVER_CMD
touch "$out/start.marker"
"$porting/run_original_client.sh" "$seconds" "${@:2}" > "$out/runner.out" 2>&1 & runner_pid=$!
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
    sudo -n nsenter -t "$server_pid" -n tcpdump -i lo -n -U -w "$out/udp.pcap" udp port 7777 > "$out/tcpdump.log" 2>&1 & capture_pid=$!
else
    echo 'Server did not appear within 40 seconds; see runner.out.' >&2
fi
wait "$runner_pid"; status=$?; runner_pid=
if [[ -n $capture_pid ]]; then sudo -n kill -INT "$capture_pid" 2>/dev/null || true; wait "$capture_pid" 2>/dev/null || true; capture_pid=; fi
[[ ! -e $out/udp.pcap ]] || sudo -n chown "$(id -u):$(id -g)" "$out/udp.pcap"
client="$porting/generated/wine-prefix/drive_c/users/${USER}/AppData/Local/FortniteGame/Saved/Logs/FortniteGame.log"
[[ ! -f $client ]] || cp "$client" "$out/client.log"
for shot in "$porting/generated/shots"/t*.png; do
    [[ -f $shot && $shot -nt $out/start.marker ]] && cp "$shot" "$out/" || true
done
printf '\nOutput directory: %s\nRunner exit status: %s\n' "$out" "$status"
# The runner translates its normal timeout into exit 0; preserve its reported status too.
grep -a 'Original client runner exit status:' "$out/runner.out" | tail -1 || true
printf 'Screenshots:'; for shot in "$out"/t*.png; do [[ ! -f $shot ]] || printf ' %s' "${shot##*/}"; done; printf '\n'
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
