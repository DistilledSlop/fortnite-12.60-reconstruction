#!/usr/bin/env bash
# Run the original Win64 client under Wine, with EAC/BattlEye disabled, inside a
# private network namespace alongside the local backend. Host names resolve to
# loopback via the prefix hosts file, so nothing reaches the real Epic services.
# Usage: run_original_client.sh [seconds] [extra client args...]
set -euo pipefail

porting_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
install_dir=$(cd -- "$porting_dir/.." && pwd)
bin_dir="$install_dir/Fortnite/FortniteGame/Binaries/Win64"
generated="$porting_dir/generated"
seconds=${1:-120}
shift || true

if [[ -z "${FN_NETNS_INNER:-}" ]]; then
    exec sudo -n env FN_NETNS_INNER=1 FN_USER="$USER" unshare -n -m "$porting_dir/run_original_client.sh" "$seconds" "$@"
fi

ip link set lo up
# Hosts and certificate are derived files, created on first use.
if [[ ! -f "$generated/hosts" ]]; then
    python3 "$porting_dir/server/gen_hosts.py" "$generated/DefaultEngine.ini" "$generated/hosts"
fi
if [[ ! -f "$generated/tls/cert.pem" ]]; then
    "$porting_dir/server/gen_tls_cert.sh" "$generated/hosts" "$generated/tls"
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
python3 "$porting_dir/server/patch_curl_verify.py" "$seconds" "$client_log" > "$generated/patch-curl-verify.out" 2>&1 &
patcher=$!
trap 'kill $server $patcher 2>/dev/null || true' EXIT

sudo -n -u "$user" env WINEPREFIX="$generated/wine-prefix" WINEDEBUG=-all \
    xvfb-run -a -s '-screen 0 1280x720x24' timeout "${seconds}s" \
    bash -c 'cd "$0" && exec wine FortniteClient-Win64-Shipping.exe "$@"' "$bin_dir" \
    -NOEPICPORTAL -nosound -NoSplash -log -epicapp=Fortnite -epicenv=Prod \
    -epiclocale=en-us -skippatchcheck -nobe -noeac -fromfl=none \
    -AUTH_LOGIN=unused -AUTH_PASSWORD=localexchangecode -AUTH_TYPE=exchangecode "$@" \
    > "$generated/original-client-run.out" 2>&1 || true
