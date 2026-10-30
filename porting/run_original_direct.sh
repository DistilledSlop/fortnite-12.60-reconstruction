#!/usr/bin/env bash
# Direct connect experiment: runs the original Win64 client under Wine and our
# native dedicated server in the same private network namespace. Status and
# evidence: porting/direct_connect_results.md. Observed so far: a bare address as
# the first command-line token is rewritten to "-address" by UE's command-line
# whitelist and ignored; a loose GameDefaultMap override did not redirect the
# client's default-map browse. The client still reaches the frontend login.
# Readiness uses the UDP socket (IpNetDriver), not TCP. The server stdout is line-buffered (stdbuf).
# No -AUTH_* flags and no matchmaking. EAC and BattlEye stay disabled with the
# same flags as run_original_client.sh. Nothing is patched in the Wine process
# or on disk: the curl-verify memory patch is deliberately not used.
# The marked block in the client's loose Saved/Config/WindowsClient/Engine.ini
# holds the net settings from original_client_net_protocol.md section 4.
# Usage: run_original_direct.sh [seconds] [extra client args...]
# Env (optional): FN_MAP (server map), FN_SERVER_ARGS (extra server args),
#   FN_NET_KEY_FILE (path to a file holding the 32-byte key; only when testing
#   encryption, never stored in the repo), FN_PORTAL_ARG, FN_EPICENV.
set -euo pipefail

porting_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
install_dir=$(cd -- "$porting_dir/.." && pwd)
bin_dir="$install_dir/Fortnite/FortniteGame/Binaries/Win64"
generated="$porting_dir/generated"
out="$generated/direct-connect"
server="$generated/linux-stage/LinuxNoEditor/FortniteLinux/Binaries/Linux/FortniteLinuxServer"
seconds=${1:-90}
shift || true
map=${FN_MAP:-/Game/Maps/FortniteEmptyDedicated}

ini="$generated/wine-prefix/drive_c/users/$USER/AppData/Local/FortniteGame/Saved/Config/WindowsClient/Engine.ini"
if [[ -z "${FN_NETNS_INNER:-}" ]]; then
    # Replace only our marked block in the loose client Engine.ini (not the EAC/AppES settings).
    python3 - "$ini" <<'PYEOF'
import sys
path = sys.argv[1]
begin, end = "; BEGIN direct-connect", "; END direct-connect"
block = "\n".join([
    begin,
    "[SystemSettings]",
    "net.MagicHeader=1110",
    "[PacketHandlerComponents]",
    "EncryptionComponent=AESHandlerComponent",
    "[OodleHandlerComponent]",
    "bEnableOodle=False",
    end,
    "",
])
text = open(path).read()
if begin in text:
    head = text[:text.index(begin)]
    tail = text[text.index(end) + len(end):].lstrip("\n")
    text = head + tail
text = text.rstrip("\n") + "\n\n" + block
open(path, "w").write(text)
PYEOF
    exec sudo -n env FN_NETNS_INNER=1 FN_USER="$USER" FN_MAP="$map" \
        FN_SERVER_ARGS="${FN_SERVER_ARGS:-}" FN_NET_KEY_FILE="${FN_NET_KEY_FILE:-}" \
        FN_PORTAL_ARG="${FN_PORTAL_ARG:--NOEPICPORTAL}" FN_EPICENV="${FN_EPICENV:-Prod}" \
        unshare -n -m "$porting_dir/run_original_direct.sh" "$seconds" "$@"
fi

if [[ ! -x "$server" ]]; then
    printf 'Stage FortniteLinuxServer first (%s missing).\n' "$server" >&2
    exit 2
fi

ip link set lo up
# Hosts and certificate are derived files, created on first use (same as run_original_client.sh).
if [[ ! -f "$generated/hosts" ]]; then
    python3 "$porting_dir/server/gen_hosts.py" "$generated/DefaultEngine.ini" "$generated/hosts"
fi
if [[ ! -f "$generated/tls/cert.pem" ]]; then
    "$porting_dir/server/gen_tls_cert.sh" "$generated/tls"
fi
chown -R "$FN_USER" "$generated/hosts" "$generated/tls"
mount --bind "$generated/hosts" /etc/hosts

user=$FN_USER
mkdir -p "$out"
chown "$user" "$out"
server_log="$out/server.log"
client_log_src="$generated/wine-prefix/drive_c/users/$user/AppData/Local/FortniteGame/Saved/Logs/FortniteGame.log"
client_log="$out/client-FortniteGame.log"
rm -f "$server_log" "$client_log" "$out/client-run.out" "$client_log_src"

# Stub HTTP/TLS endpoints keep the client's background requests from waiting on DNS.
# No login flow is started because no -AUTH_* argument is passed.
python3 "$porting_dir/server/probe_server.py" --port 443 --stub \
    --tls-cert "$generated/tls/cert.pem" --tls-key "$generated/tls/key.pem" \
    --record "$out/server-requests.jsonl" > "$out/probe-server.out" 2>&1 &
probe=$!

server_env=()
if [[ -n "${FN_NET_KEY_FILE:-}" ]]; then
    server_env=(FORTNITE_NET_ENCRYPTION_KEY_FILE="$FN_NET_KEY_FILE")
fi
server_args=()
if [[ -n "${FN_SERVER_ARGS:-}" ]]; then
    read -r -a server_args <<< "$FN_SERVER_ARGS"
fi

server_ue_log="$generated/linux-stage/LinuxNoEditor/FortniteLinux/Saved/Logs/FortniteLinux.log"
server_pid=
cleanup() {
    if [[ -n "$server_pid" ]]; then
        kill "$server_pid" 2>/dev/null || true
        # sudo relays TERM to the server, but stop the binary directly as well (only this stage's binary).
        sudo -n -u "$user" pkill -TERM -f -- "$server" 2>/dev/null || true
    fi
    kill "$probe" 2>/dev/null || true
    # Bounded wait on our own children only (never a bare `wait`, which could block on anything else).
    wait "$server_pid" "$probe" 2>/dev/null || true
    # The engine's own log (separate from the stdout capture above) is kept for the record.
    if [[ -f "$server_ue_log" ]]; then
        cp "$server_ue_log" "$out/server-UE.log" 2>/dev/null || true
        chown "$user" "$out/server-UE.log" 2>/dev/null || true
    fi
}
trap cleanup EXIT

# stdbuf -oL -eL: UE writes stdout through libc, so line buffering makes the captured log
# current while the server runs. The engine also writes $server_ue_log (see cleanup).
sudo -n -u "$user" env "${server_env[@]}" timeout "$((seconds + 60))s" \
    stdbuf -oL -eL "$server" "$FN_MAP" "${server_args[@]}" -nullrhi -nosound -unattended -port=7777 -log \
    -FortniteOriginalClient > "$server_log" 2>&1 &
server_pid=$!

# Readiness is checked on the UDP socket: IpNetDriver is UDP, so a TCP listen check
# (`ss -ltn`) never sees port 7777. Falls back to /proc/net/udp{,6} when `ss` is absent.
# The server can take a while to reach listen under load (Ghidra shares the host CPU).
listening() {
    if command -v ss >/dev/null 2>&1; then
        ss -lunH 2>/dev/null | awk '$4 ~ /:7777$/ { found = 1 } END { exit !found }'
    else
        awk '$2 ~ /:1E61$/ { found = 1 } END { exit !found }' /proc/net/udp /proc/net/udp6 2>/dev/null
    fi
}
for _ in {1..900}; do
    if listening; then
        break
    fi
    kill -0 "$server_pid" 2>/dev/null || break
    sleep 0.2
done
if ! listening; then
    printf 'Server did not listen on UDP port 7777. See %s\n' "$server_log" >&2
    tail -n 20 "$server_log" >&2 || true
    exit 1
fi
printf 'Server listening on UDP 7777.\n' >&2

# Client flags match run_original_client.sh minus the AUTH flags; the target comes from the Engine.ini block.
sudo -n -u "$user" env WINEPREFIX="$generated/wine-prefix" WINEDEBUG=-all \
    xvfb-run -a -s '-screen 0 1280x720x24' timeout "${seconds}s" \
    bash -c 'cd "$0" && exec wine FortniteClient-Win64-Shipping.exe "$@"' "$bin_dir" \
    "$FN_PORTAL_ARG" -nosound -NoSplash -log -epiclocale=en-us \
    -skippatchcheck -nobe -noeac -fromfl=none -epicapp=Fortnite -epicenv="$FN_EPICENV" "$@" \
    > "$out/client-run.out" 2>&1 || true

if [[ -f "$client_log_src" ]]; then
    cp "$client_log_src" "$client_log"
    chown "$user" "$client_log"
fi
# Stop only this prefix's Wine server; the Ghidra and other host processes are untouched.
sudo -n -u "$user" env WINEPREFIX="$generated/wine-prefix" wineserver -k >/dev/null 2>&1 || true
printf 'Direct run finished. Server log: %s Client log: %s\n' "$server_log" "$client_log"
