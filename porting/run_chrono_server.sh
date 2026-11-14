#!/usr/bin/env bash
set -euo pipefail

porting=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
generated="$porting/generated"

backend_bind=${FN_BACKEND_BIND:-0.0.0.0}
backend_port=${FN_BACKEND_PORT:-8443}
gameserver_port=${FORTNITE_GAMESERVER_PORT:-7777}
gameserver_beacon_port=${FORTNITE_GAMESERVER_BEACON_PORT:-7778}
gameserver_address=${FORTNITE_GAMESERVER_ADDRESS:-127.0.0.1}
gameserver_playlist=${FORTNITE_GAMESERVER_PLAYLIST:-Playlist_DefaultSolo}
gameserver_build=${FORTNITE_GAMESERVER_BUILD:-12896485}
gameserver_capacity=${FORTNITE_GAMESERVER_CAPACITY:-100}
server_map=${FN_SERVER_MAP:-/Game/Athena/Apollo/Maps/Apollo_Terrain}
server_url_opts=${FN_SERVER_URL_OPTS:-?RequiredPlayers=1}
state_dir=${FN_STATE_DIR:-$generated/chrono-server-state}
log_dir=${FN_LOG_DIR:-$generated/chrono-server-logs}
server_log_cmds=${FN_SERVER_LOG_CMDS:-LogHandshake Log, LogNet Log, LogNetTraffic Warning, PacketHandlerLog Log}

tls_cert="$generated/tls/cert.pem"
tls_key="$generated/tls/key.pem"
linux_server="$generated/linux-stage/LinuxNoEditor/FortniteLinux/Binaries/Linux/FortniteLinuxServer"

for req in "$tls_cert" "$tls_key" "$linux_server"; do
    if [[ ! -f $req ]]; then
        printf 'Missing required file: %s\n' "$req" >&2
        exit 1
    fi
done

mkdir -p "$state_dir" "$log_dir" "$generated/gameserver-secrets"
chmod 700 "$generated/gameserver-secrets"

gs_http_port=$(python3 -c '
import socket
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind(("127.0.0.1", 0))
    print(s.getsockname()[1])
')

secret_file=$(python3 - "$generated/gameserver-secrets" <<'PYEOF'
import os, secrets, sys, tempfile
fd, path = tempfile.mkstemp(prefix="chrono-secret-", dir=sys.argv[1])
try:
    os.fchmod(fd, 0o600)
    os.write(fd, secrets.token_hex(32).encode("ascii"))
finally:
    os.close(fd)
print(path)
PYEOF
)

backend_pid=
gameserver_pid=
cleanup() {
    local rc=$?
    for pid in ${gameserver_pid:-} ${backend_pid:-}; do
        if [[ -n $pid ]] && kill -0 "$pid" 2>/dev/null; then
            kill -TERM "$pid" 2>/dev/null || true
        fi
    done
    for pid in ${gameserver_pid:-} ${backend_pid:-}; do
        if [[ -n $pid ]]; then
            wait "$pid" 2>/dev/null || true
        fi
    done
    rm -f -- "$secret_file"
    exit "$rc"
}
trap cleanup EXIT INT TERM

IFS= read -r gs_secret < "$secret_file" || [[ -n "${gs_secret:-}" ]]
FORTNITE_GAMESERVER_SECRET="$gs_secret" python3 "$porting/server/probe_server.py" \
    --bind "$backend_bind" \
    --port "$backend_port" \
    --stub \
    --state-dir "$state_dir" \
    --tls-cert "$tls_cert" \
    --tls-key "$tls_key" \
    --registered-matchmaking \
    --local-gameserver-http-port "$gs_http_port" \
    --record "$log_dir/server-requests.jsonl" \
    > "$log_dir/backend.log" 2>&1 &
backend_pid=$!

if ! python3 - "$backend_pid" "$gs_http_port" "$backend_port" <<'PYEOF'
import http.client, os, ssl, sys, time
pid, http_port, tls_port = int(sys.argv[1]), int(sys.argv[2]), int(sys.argv[3])
deadline = time.monotonic() + 10.0
tls_ctx = ssl._create_unverified_context()
def alive():
    try:
        st = open(f"/proc/{pid}/stat", encoding="utf-8", errors="replace").read()
        return st.rsplit(")", 1)[1].split()[0] != "Z"
    except OSError:
        return False
def healthy(conn):
    try:
        conn.request("GET", "/__health")
        return conn.getresponse().status == 200
    finally:
        conn.close()
while time.monotonic() < deadline:
    if not alive():
        sys.exit(1)
    try:
        if (healthy(http.client.HTTPConnection("127.0.0.1", http_port, timeout=0.5)) and
                healthy(http.client.HTTPSConnection("127.0.0.1", tls_port, timeout=0.5, context=tls_ctx))):
            sys.exit(0)
    except OSError:
        pass
    time.sleep(0.1)
sys.exit(1)
PYEOF
then
    unset gs_secret
    printf 'Backend failed to become healthy on port %s (see %s/backend.log).\n' "$backend_port" "$log_dir" >&2
    exit 1
fi

read -r -a extra_args <<< "${FN_SERVER_ARGS:--FortniteSkipUnconvertedLegacyShaderMaps -FortniteTolerateNeedLoadDependencies -FortniteSkipEngineClassNetChecksum -FortniteStartingWood=500 -FortniteBlueprintPawn -FortniteNoServerAnimDecode -FortniteInventoryAbilities -FortniteWarmupSeconds=90}"

FORTNITE_GAMESERVER_URL="http://127.0.0.1:$gs_http_port" \
FORTNITE_GAMESERVER_SECRET="$gs_secret" \
FORTNITE_GAMESERVER_ADDRESS="$gameserver_address" \
FORTNITE_GAMESERVER_PLAYLIST="$gameserver_playlist" \
FORTNITE_GAMESERVER_BUILD="$gameserver_build" \
FORTNITE_GAMESERVER_PORT="$gameserver_port" \
FORTNITE_GAMESERVER_BEACON_PORT="$gameserver_beacon_port" \
FORTNITE_GAMESERVER_CAPACITY="$gameserver_capacity" \
"$linux_server" \
    "${server_map}${server_url_opts}" \
    -FortniteRegisteredMatchmaking \
    -FortniteOriginalClient \
    -nullrhi \
    -nosound \
    -unattended \
    "-port=$gameserver_port" \
    -log \
    "-LogCmds=$server_log_cmds" \
    "${extra_args[@]}" \
    > "$log_dir/gameserver.log" 2>&1 &
gameserver_pid=$!
unset gs_secret

printf 'ProjectChrono server stack started:\n'
printf '  Backend TLS:  %s:%s (loopback gameserver API: 127.0.0.1:%s)\n' "$backend_bind" "$backend_port" "$gs_http_port"
printf '  Game UDP:     0.0.0.0:%s (advertised: %s:%s)\n' "$gameserver_port" "$gameserver_address" "$gameserver_port"
printf '  Map:          %s%s\n' "$server_map" "$server_url_opts"
printf '  State dir:    %s\n' "$state_dir"
printf '  Logs:         %s/backend.log , %s/gameserver.log\n' "$log_dir" "$log_dir"

while kill -0 "$backend_pid" 2>/dev/null && kill -0 "$gameserver_pid" 2>/dev/null; do
    sleep 1
done

printf 'A server process exited (see %s/backend.log and %s/gameserver.log).\n' "$log_dir" "$log_dir" >&2
exit 1
