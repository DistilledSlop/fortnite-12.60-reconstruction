#!/usr/bin/env bash
# Run the original Win64 client under Wine, with EAC/BattlEye disabled, inside a
# private network namespace alongside the local backend. Host names resolve to
# loopback via the prefix hosts file, so nothing reaches the real Epic services.
# Usage: run_original_client.sh [--username NAME] [seconds] [extra client args...]
# FN_ACTIONS="60:shot 70:click,640,600 80:key,Return" runs X11 actions at those seconds after
# launch; screenshots go to generated/shots/ (debug aids, never committed).
set -euo pipefail

porting_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
install_dir=$(cd -- "$porting_dir/.." && pwd)
bin_dir="$install_dir/Fortnite/FortniteGame/Binaries/Win64"
generated="$porting_dir/generated"
username=${FN_USERNAME:-LocalPlayer}
if [[ "${1:-}" == --username ]]; then
    if [[ $# -lt 2 || -z "$2" ]]; then
        printf "Usage: %s [--username NAME] [seconds] [extra client args...]\n" "$0" >&2
        exit 2
    fi
    username=$2
    shift 2
fi
seconds=${1:-120}
shift || true

if [[ -z "${FN_NETNS_INNER:-}" ]]; then
    exec sudo -n env FN_NETNS_INNER=1 FN_USER="$USER" FN_USERNAME="$username" FN_PATCH_VERIFY="${FN_PATCH_VERIFY:-1}" FN_ACTIONS="${FN_ACTIONS:-}" FN_PORTAL_ARG="${FN_PORTAL_ARG:--NOEPICPORTAL}" FN_EPICENV="${FN_EPICENV:-Prod}" unshare -n -m "$porting_dir/run_original_client.sh" "$seconds" "$@"
fi

ip link set lo up
# Hosts and certificate are derived files, created on first use.
if [[ ! -f "$generated/hosts" ]]; then
    python3 "$porting_dir/server/gen_hosts.py" "$generated/DefaultEngine.ini" "$generated/hosts"
fi
if [[ ! -f "$generated/tls/cert.pem" ]]; then
    "$porting_dir/server/gen_tls_cert.sh" "$generated/tls"
fi
chown -R "$FN_USER" "$generated/hosts" "$generated/tls"
mount --bind "$generated/hosts" /etc/hosts
user=$FN_USER
client_log="$generated/wine-prefix/drive_c/users/$user/AppData/Local/FortniteGame/Saved/Logs/FortniteGame.log"
rm -f "$client_log" "$generated/server-requests.jsonl"
python3 "$porting_dir/server/probe_server.py" --port 443 --stub \
    --tls-cert "$generated/tls/cert.pem" --tls-key "$generated/tls/key.pem" \
    > "$generated/probe-server.out" 2>&1 &
server=$!
patcher=
if [[ "$FN_PATCH_VERIFY" == 1 ]]; then
    python3 "$porting_dir/server/patch_curl_verify.py" "$seconds" "$client_log" > "$generated/patch-curl-verify.out" 2>&1 &
    patcher=$!
fi
trap 'kill $server $patcher 2>/dev/null || true' EXIT

mkdir -p "$generated/shots"
chown "$FN_USER" "$generated/shots"
sudo -n -u "$user" env WINEPREFIX="$generated/wine-prefix" WINEDEBUG=-all SHOTS_DIR="$generated/shots" FN_ACTIONS="$FN_ACTIONS" \
    xvfb-run -a -s '-screen 0 1280x720x24' timeout "${seconds}s" \
    bash -c '
cd "$0"
if [[ -n "$FN_ACTIONS" ]]; then
    (
        prev=0
        for action in $FN_ACTIONS; do
            at=${action%%:*}; what=${action#*:}; kind=${what%%,*}; args=${what#*,}
            sleep $(( at > prev ? at - prev : 0 )); prev=$at
            case $kind in
                shot) ffmpeg -loglevel error -y -f x11grab -video_size 1280x720 -i "$DISPLAY" -frames:v 1 "$SHOTS_DIR/t$at.png" ;;
                click) xdotool mousemove ${args%%,*} ${args#*,} click 1 ;;
                key) xdotool key "$args" ;;
            esac
        done
    ) &
fi
exec wine FortniteClient-Win64-Shipping.exe "$@"' "$bin_dir" \
    "$FN_PORTAL_ARG" -nosound -NoSplash -log -epicapp=Fortnite -epicenv="$FN_EPICENV" \
    -epiclocale=en-us -skippatchcheck -nobe -noeac -fromfl=none \
    -AUTH_LOGIN=unused "-AUTH_PASSWORD=local-username:$username" -AUTH_TYPE=exchangecode "$@" \
    > "$generated/original-client-run.out" 2>&1 || true
