#!/usr/bin/env bash
# Run the original Win64 client under Wine, with EAC/BattlEye disabled, inside a
# private network namespace alongside the local backend. Host names resolve to
# loopback via the prefix hosts file, so nothing reaches the real Epic services.
# Usage: run_original_client.sh [--username NAME] [seconds] [extra client args...]
# FN_ACTIONS="60:shot 70:click,640,600 80:key,Return" runs X11 actions at those seconds after
# launch; "wait,REGEX[,TIMEOUT]" waits for REGEX in the client or FN_ACTION_LOG and restarts the clock;
# also stopif,REGEX / move,X,Y / look,DX,DY / mhold,SECONDS. Screenshots go to $FN_SHOTS_DIR (vanilla_play_run.sh: the run folder's shots/), else generated/runs/_unsorted-shots/.
# FN_WINEPREFIX selects an alternate prefix; FN_WINEDLLOVERRIDES and
# FN_VK_ICD_FILENAMES pass renderer settings into the isolated client.
# Set FN_DISPLAY and, if required, FN_XAUTHORITY to use an existing X11 display.
# Without FN_DISPLAY, the runner creates a private Xvfb display.
# FN_STATE_DIR selects a separate local backend account/profile directory.
# FN_WALLET_TRACE writes opt-in redacted wallet request/response diagnostics.
# Region QoS uses a measured loopback echo on UDP 22222 by default.
# Set FN_LOCAL_QOS_PORT to an empty value to disable it, or another UDP port to override.
# FN_SERVER_CMD starts a command (for example the dedicated server) in the same namespace.
# FN_REGISTERED_MATCHMAKING=1 enables opt-in registered server matchmaking using a
# loopback HTTP port inside the namespace and FORTNITE_GAMESERVER_SECRET_FILE (0600).
set -euo pipefail

porting_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
install_dir=$(cd -- "$porting_dir/.." && pwd)
bin_dir="$install_dir/Fortnite/FortniteGame/Binaries/Win64"
generated="$porting_dir/generated"
wineprefix=${FN_WINEPREFIX:-"$generated/wine-prefix"}
username=${FN_USERNAME:-LocalPlayer}
if [[ "${1:-}" == --username ]]; then
    if [[ $# -lt 2 || -z "$2" ]]; then
        printf "Usage: %s [--username NAME] [seconds] [extra client args...]\n" "$0" >&2
        exit 2
    fi
    username=$2
    shift 2
fi

validate_registered_config() {
    local mode=${FN_REGISTERED_MATCHMAKING:-0}
    if [[ "$mode" != 0 && "$mode" != 1 ]]; then
        printf 'Invalid FN_REGISTERED_MATCHMAKING=%s (expected 0 or 1).\n' "$mode" >&2
        exit 2
    fi
    [[ "$mode" == 1 ]] || return 0
    if [[ -n "${FORTNITE_GAMESERVER_URL:-}" ]]; then
        printf 'FORTNITE_GAMESERVER_URL is assigned by the runner; do not set it externally.\n' >&2
        exit 2
    fi
    if [[ -n "${FORTNITE_GAMESERVER_SECRET:-}" ]]; then
        printf 'Set FORTNITE_GAMESERVER_SECRET_FILE instead of FORTNITE_GAMESERVER_SECRET.\n' >&2
        exit 2
    fi
    local target_user=${1:-$USER}
    python3 - "$target_user" \
        "${FORTNITE_GAMESERVER_ADDRESS:-127.0.0.1}" \
        "${FORTNITE_GAMESERVER_PLAYLIST:-Playlist_DefaultSolo}" \
        "${FORTNITE_GAMESERVER_BUILD:-12896485}" \
        "${FORTNITE_GAMESERVER_PORT:-7777}" \
        "${FORTNITE_GAMESERVER_BEACON_PORT:-7778}" \
        "${FORTNITE_GAMESERVER_CAPACITY:-100}" \
        "${FORTNITE_GAMESERVER_SECRET_FILE:-}" \
        "${FN_BACKEND_PORT:-443}" <<'PYEOF'
import os, pwd, stat, sys
user, addr, playlist, build, port_s, beacon_s, cap_s, secret_file, backend_port_s = sys.argv[1:10]
def bad(msg):
    print(msg, file=sys.stderr)
    sys.exit(2)
def check_str(name, val, maxlen):
    if not val or len(val) > maxlen or any(c.isspace() or ord(c) < 32 for c in val):
        bad(f"Invalid {name}.")
def check_int(name, val, lo, hi):
    if not val.isdigit() or len(val) > 5:
        bad(f"Invalid {name}.")
    n = int(val)
    if n < lo or n > hi:
        bad(f"Invalid {name}.")
check_str("FORTNITE_GAMESERVER_ADDRESS", addr, 255)
check_str("FORTNITE_GAMESERVER_PLAYLIST", playlist, 255)
check_str("FORTNITE_GAMESERVER_BUILD", build, 128)
check_int("FORTNITE_GAMESERVER_PORT", port_s, 1, 65535)
check_int("FORTNITE_GAMESERVER_BEACON_PORT", beacon_s, 1, 65535)
check_int("FORTNITE_GAMESERVER_CAPACITY", cap_s, 1, 1000)
check_int("FN_BACKEND_PORT", backend_port_s, 1, 65535)
if secret_file:
    try:
        uid = pwd.getpwnam(user).pw_uid
    except KeyError:
        bad(f"Unknown runner user: {user}")
    try:
        st = os.lstat(secret_file)
    except OSError:
        bad("FORTNITE_GAMESERVER_SECRET_FILE is missing or inaccessible.")
    if stat.S_ISLNK(st.st_mode) or not stat.S_ISREG(st.st_mode):
        bad("FORTNITE_GAMESERVER_SECRET_FILE must be a regular file (not a symlink).")
    if st.st_uid != uid:
        bad("FORTNITE_GAMESERVER_SECRET_FILE must be owned by the runner user.")
    if (st.st_mode & 0o077) != 0 or (st.st_mode & 0o400) == 0:
        bad("FORTNITE_GAMESERVER_SECRET_FILE must not be group/other-accessible (expected mode 0600 or 0400).")
    try:
        fd = os.open(secret_file, os.O_RDONLY | getattr(os, "O_NOFOLLOW", 0))
        try:
            fst = os.fstat(fd)
            if fst.st_dev != st.st_dev or fst.st_ino != st.st_ino or not stat.S_ISREG(fst.st_mode):
                bad("FORTNITE_GAMESERVER_SECRET_FILE changed during validation.")
            raw = os.read(fd, 4097)
        finally:
            os.close(fd)
    except OSError:
        bad("Cannot read FORTNITE_GAMESERVER_SECRET_FILE.")
    if not raw or len(raw) > 4096:
        bad("FORTNITE_GAMESERVER_SECRET_FILE must be non-empty and at most 4096 bytes.")
    if b"\r" in raw or b"\n" in raw or b"\0" in raw:
        bad("FORTNITE_GAMESERVER_SECRET_FILE must not contain CR, LF, or NUL.")
    try:
        text = raw.decode("utf-8")
    except UnicodeDecodeError:
        bad("FORTNITE_GAMESERVER_SECRET_FILE must be valid UTF-8.")
    if not text.strip():
        bad("FORTNITE_GAMESERVER_SECRET_FILE must not be whitespace-only.")
PYEOF
}

if [[ "${1:-}" == --validate-registered ]]; then
    validate_registered_config "$USER"
    exit 0
fi
seconds=${1:-120}
shift || true

if [[ -z "${FN_NETNS_INNER:-}" ]]; then
    validate_registered_config "$USER"
    # A server retained by this prefix can create Wine sockets in an earlier
    # network namespace, silently disconnecting a new client from its backend.
    wine_server=${FN_WINESERVER:-$(command -v wineserver || true)}
    if [[ -z "$wine_server" && -x /usr/lib/x86_64-linux-gnu/wine/wineserver ]]; then
        wine_server=/usr/lib/x86_64-linux-gnu/wine/wineserver
    fi
    if [[ -z "$wine_server" || ! -x "$wine_server" ]]; then
        printf 'Cannot find wineserver; set FN_WINESERVER to its executable path.\n' >&2
        exit 1
    fi
    if ! WINEPREFIX="$wineprefix" timeout 5 "$wine_server" -w; then
        printf 'Wine is still using prefix %s. Close its Wine applications and stop its wineserver before retrying.\n' "$wineprefix" >&2
        exit 1
    fi
    # Net settings the original client needs against our server (Engine.ini is a loose config file, the exe is untouched):
    # the client's Oodle handler adds a "compressed" bit to every packet; we have no Oodle SDK, so it must never compress.
    ini="$wineprefix/drive_c/users/$USER/AppData/Local/FortniteGame/Saved/Config/WindowsClient/Engine.ini"
    if [[ -d "$(dirname "$ini")" ]]; then
        python3 - "$ini" <<'PYEOF'
import sys,re
path=sys.argv[1]
begin,end="; BEGIN original-client-net","; END original-client-net"
block="\n".join([begin,"[SystemSettings]","net.OodleMinSizeForCompression=100000","[ConsoleVariables]","net.OodleMinSizeForCompression=100000",end])+"\n"
try:
    text=open(path).read()
except FileNotFoundError:
    text=""
text=re.sub(re.escape(begin)+r".*?"+re.escape(end)+r"\n?","",text,flags=re.S)
if text and not text.endswith("\n"): text+="\n"
open(path,"w").write(text+block)
PYEOF
    fi
    exec sudo -n env \
        FN_CLIENT_RUN_LOG="${FN_CLIENT_RUN_LOG:-}" FN_XVFB_ERROR_LOG="${FN_XVFB_ERROR_LOG:-}" \
        FN_NETNS_INNER=1 FN_SESSION_TRACE="${FN_SESSION_TRACE:-0}" FN_REGISTERED_MATCHMAKING="${FN_REGISTERED_MATCHMAKING:-0}" FN_BACKEND_PORT="${FN_BACKEND_PORT:-443}" FN_LOCAL_QOS_PORT="${FN_LOCAL_QOS_PORT-22222}" FN_WALLET_TRACE="${FN_WALLET_TRACE:-}" FN_USER="$USER" FN_USERNAME="$username" FN_WINEPREFIX="$wineprefix" FN_WINEDLLOVERRIDES="${FN_WINEDLLOVERRIDES:-}" FN_VK_ICD_FILENAMES="${FN_VK_ICD_FILENAMES:-}" FN_LP_NUM_THREADS="${FN_LP_NUM_THREADS:-}" FN_PATCH_VERIFY="${FN_PATCH_VERIFY:-1}" FN_PATCH_APPES_DIAGNOSTIC="${FN_PATCH_APPES_DIAGNOSTIC:-0}" FN_ACTIONS="${FN_ACTIONS:-}" FN_ACTION_LOG="${FN_ACTION_LOG:-}" FN_SHOTS_DIR="${FN_SHOTS_DIR:-}" FORTNITE_NET_ENCRYPTION_KEY_FILE="${FORTNITE_NET_ENCRYPTION_KEY_FILE:-}" FORTNITE_GAMESERVER_SECRET_FILE="${FORTNITE_GAMESERVER_SECRET_FILE:-}" FORTNITE_GAMESERVER_ADDRESS="${FORTNITE_GAMESERVER_ADDRESS:-}" FORTNITE_GAMESERVER_PLAYLIST="${FORTNITE_GAMESERVER_PLAYLIST:-}" FORTNITE_GAMESERVER_BUILD="${FORTNITE_GAMESERVER_BUILD:-}" FORTNITE_GAMESERVER_PORT="${FORTNITE_GAMESERVER_PORT:-}" FORTNITE_GAMESERVER_BEACON_PORT="${FORTNITE_GAMESERVER_BEACON_PORT:-}" FORTNITE_GAMESERVER_CAPACITY="${FORTNITE_GAMESERVER_CAPACITY:-}" FN_STATE_DIR="${FN_STATE_DIR:-}" FN_SERVER_CMD="${FN_SERVER_CMD:-}" FN_WINEDEBUG="${FN_WINEDEBUG:--all}" FN_DISPLAY="${FN_DISPLAY:-}" FN_XAUTHORITY="${FN_XAUTHORITY:-}" FN_PORTAL_ARG="${FN_PORTAL_ARG:--NOEPICPORTAL}" FN_EPICENV="${FN_EPICENV:-Prod}" unshare -n -m "$porting_dir/run_original_client.sh" "$seconds" "$@"
fi

ip link set lo up
validate_registered_config "$FN_USER"
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
client_log="$wineprefix/drive_c/users/$user/AppData/Local/FortniteGame/Saved/Logs/FortniteGame.log"
rm -f "$client_log" "$generated/server-requests.jsonl"
state_args=()
[[ -z "${FN_STATE_DIR:-}" ]] || state_args=(--state-dir "$FN_STATE_DIR")
backend_port=${FN_BACKEND_PORT:-443}
qos_args=()
qos_port=${FN_LOCAL_QOS_PORT-22222}
[[ -z "$qos_port" ]] || qos_args=(--local-qos-port "$qos_port")
wallet_trace_args=()
[[ -z "${FN_WALLET_TRACE:-}" ]] || wallet_trace_args=(--wallet-trace "$FN_WALLET_TRACE")
server=
patcher=
appes_patcher=
oodle_patcher=
server_cmd_pid=
client_pid=
ephemeral_secret_file=
cleanup() {
    local pid
    # Capture group members before signalling, then verify a surviving member's
    # start time before escalating, so a recycled group ID is not targeted.
    python3 - ${client_pid:-} ${server_cmd_pid:-} <<'PYEOF'
import os, signal, sys, time

def members(groups):
    found = {}
    for name in os.listdir("/proc"):
        if not name.isdigit():
            continue
        try:
            with open(f"/proc/{name}/stat") as stat:
                fields = stat.read().rsplit(")", 1)[1].split()
            state, group, start = fields[0], int(fields[2]), fields[19]
            if group in groups and state != "Z":
                found[int(name)] = (group, start)
        except (OSError, ValueError, IndexError):
            pass
    return found

groups = {int(arg) for arg in sys.argv[1:]}
original = members(groups)
for group in groups & {item[0] for item in original.values()}:
    try:
        os.killpg(group, signal.SIGTERM)
    except ProcessLookupError:
        pass
if original:
    time.sleep(0.3)
    current = members(groups)
    surviving_groups = {group for pid, (group, start) in current.items()
                        if original.get(pid) == (group, start)}
    for group in surviving_groups:
        try:
            os.killpg(group, signal.SIGKILL)
        except ProcessLookupError:
            pass
PYEOF
    for pid in ${server:-} ${patcher:-} ${appes_patcher:-} ${oodle_patcher:-}; do
        kill "$pid" 2>/dev/null || true
    done
    if [[ -n "${ephemeral_secret_file:-}" ]]; then
        rm -f -- "$ephemeral_secret_file"
    fi
}
trap cleanup EXIT
trap 'exit 143' TERM
trap 'exit 130' INT

gameserver_secret_file=
gameserver_url=
gameserver_address=
gameserver_playlist=
gameserver_build=
gameserver_port=
gameserver_beacon_port=
gameserver_capacity=
if [[ "${FN_REGISTERED_MATCHMAKING:-0}" == 1 ]]; then
    unset FORTNITE_NET_ENCRYPTION_KEY FORTNITE_NET_ENCRYPTION_KEY_FILE
    gameserver_address=${FORTNITE_GAMESERVER_ADDRESS:-127.0.0.1}
    gameserver_playlist=${FORTNITE_GAMESERVER_PLAYLIST:-Playlist_DefaultSolo}
    gameserver_build=${FORTNITE_GAMESERVER_BUILD:-12896485}
    gameserver_port=${FORTNITE_GAMESERVER_PORT:-7777}
    gameserver_beacon_port=${FORTNITE_GAMESERVER_BEACON_PORT:-7778}
    gameserver_capacity=${FORTNITE_GAMESERVER_CAPACITY:-100}
    gameserver_secret_file=${FORTNITE_GAMESERVER_SECRET_FILE:-}
    ws_port=$(python3 -c '
import socket
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind(("127.0.0.1", 0))
    print(s.getsockname()[1])
')
    if [[ ! "$ws_port" =~ ^[1-9][0-9]*$ ]] || (( ws_port > 65535 )); then
        printf 'Failed to select a free loopback HTTP port.\n' >&2
        exit 1
    fi
    gameserver_url="http://127.0.0.1:$ws_port"
    if [[ -z "$gameserver_secret_file" ]]; then
        secret_dir="$generated/gameserver-secrets"
        mkdir -p "$secret_dir"
        chmod 700 "$secret_dir"
        chown "$user" "$secret_dir"
        ephemeral_secret_file=$(python3 - "$secret_dir" <<'PYEOF'
import os, secrets, sys, tempfile
fd, path = tempfile.mkstemp(prefix="secret-", dir=sys.argv[1])
try:
    os.fchmod(fd, 0o600)
    os.write(fd, secrets.token_hex(32).encode("ascii"))
finally:
    os.close(fd)
print(path)
PYEOF
)
        chown "$user" "$ephemeral_secret_file"
        gameserver_secret_file=$ephemeral_secret_file
    fi
    IFS= read -r gs_secret < "$gameserver_secret_file" || [[ -n "${gs_secret:-}" ]]
    FORTNITE_GAMESERVER_SECRET="$gs_secret" python3 "$porting_dir/server/probe_server.py" \
        --port "$backend_port" --stub "${state_args[@]}" "${qos_args[@]}" "${wallet_trace_args[@]}" \
        --tls-cert "$generated/tls/cert.pem" --tls-key "$generated/tls/key.pem" \
        --registered-matchmaking --local-matchmaking-ws-port "$ws_port" \
        > "$generated/probe-server.out" 2>&1 &
    server=$!
    unset gs_secret
    if ! python3 - "$server" "$ws_port" "$backend_port" <<'PYEOF'
import http.client, ssl, sys, time
pid, ws_port, tls_port = int(sys.argv[1]), int(sys.argv[2]), int(sys.argv[3])
deadline = time.monotonic() + 10.0
tls_ctx = ssl._create_unverified_context()
def alive():
    try:
        stat_text = open(f"/proc/{pid}/stat", "r", encoding="utf-8", errors="replace").read()
        return stat_text.rsplit(")", 1)[1].split()[0] != "Z"
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
        if (healthy(http.client.HTTPConnection("127.0.0.1", ws_port, timeout=0.5)) and
                healthy(http.client.HTTPSConnection("127.0.0.1", tls_port, timeout=0.5, context=tls_ctx))):
            sys.exit(0)
    except OSError:
        pass
    time.sleep(0.05)
sys.exit(1)
PYEOF
    then
        printf 'Registered matchmaking backend listeners failed to become ready (see %s).\n' "$generated/probe-server.out" >&2
        exit 1
    fi
    printf 'Registered matchmaking trace: FN_REGISTERED_MATCHMAKING=1 loopback_port=%s secret_file=%s\n' "$ws_port" "$gameserver_secret_file"
else
    python3 "$porting_dir/server/probe_server.py" --port "$backend_port" --stub "${state_args[@]}" "${qos_args[@]}" "${wallet_trace_args[@]}" \
        --tls-cert "$generated/tls/cert.pem" --tls-key "$generated/tls/key.pem" \
        --local-matchmaking-ws-port 0 \
        > "$generated/probe-server.out" 2>&1 &
    server=$!
fi
if [[ "$FN_PATCH_VERIFY" == 1 ]]; then
    python3 "$porting_dir/server/patch_curl_verify.py" "$seconds" "$client_log" > "$generated/patch-curl-verify.out" 2>&1 &
    patcher=$!
fi
if [[ "${FN_PATCH_APPES_DIAGNOSTIC:-0}" =~ ^(1|timer|handler-only|timer-only)$ ]]; then
    appes_mode=$FN_PATCH_APPES_DIAGNOSTIC
    [[ "$appes_mode" != 1 ]] || appes_mode=callback
    python3 "$porting_dir/server/patch_appes_diagnostic.py" "$seconds" "$client_log" "$appes_mode" > "$generated/patch-appes-diagnostic.out" 2>&1 &
    appes_patcher=$!
fi
if [[ "${FN_PATCH_OODLE:-1}" == 1 ]]; then
    # The original client's Oodle handler would compress packets we cannot decode (no Oodle SDK); see the script header.
    python3 "$porting_dir/server/patch_oodle_min_size.py" "$seconds" > "$generated/patch-oodle-min-size.out" 2>&1 &
    oodle_patcher=$!
fi

shots_dir=${FN_SHOTS_DIR:-$generated/runs/_unsorted-shots}
mkdir -p "$shots_dir"
chown "$FN_USER" "$shots_dir"
set +e
wine_env=(WINEPREFIX="$wineprefix" WINEDEBUG="${FN_WINEDEBUG:--all}" SHOTS_DIR="$shots_dir" FN_ACTIONS="$FN_ACTIONS" FN_ACTION_LOG="${FN_ACTION_LOG:-}")
[[ -z "${FN_WINEDLLOVERRIDES:-}" ]] || wine_env+=(WINEDLLOVERRIDES="$FN_WINEDLLOVERRIDES")
[[ -z "${FN_VK_ICD_FILENAMES:-}" ]] || wine_env+=(VK_ICD_FILENAMES="$FN_VK_ICD_FILENAMES")
# lavapipe defaults to one rasteriser thread per core; on a 96-core host its llvmpipe threads segfault intermittently.
[[ -z "${FN_LP_NUM_THREADS:-}" ]] || wine_env+=(LP_NUM_THREADS="$FN_LP_NUM_THREADS")
if [[ -n "${FN_DISPLAY:-}" ]]; then
    wine_env+=(DISPLAY="$FN_DISPLAY")
    [[ -z "${FN_XAUTHORITY:-}" ]] || wine_env+=(XAUTHORITY="$FN_XAUTHORITY")
    display_cmd=()
else
    display_cmd=(xvfb-run -a -s '-screen 0 1280x720x24')
    [[ -z "${FN_XVFB_ERROR_LOG:-}" ]] || display_cmd+=(-e "$FN_XVFB_ERROR_LOG")
fi
# FN_SERVER_CMD: optional command (run as the user, inside this namespace) started before the
# client, e.g. the dedicated server. Stopped when the client run ends.
if [[ -n "${FN_SERVER_CMD:-}" ]]; then
    if [[ "${FN_REGISTERED_MATCHMAKING:-0}" == 1 ]]; then
        setsid sudo -n -u "$user" env \
            FORTNITE_GAMESERVER_URL="$gameserver_url" \
            FORTNITE_GAMESERVER_SECRET_FILE="$gameserver_secret_file" \
            FORTNITE_GAMESERVER_ADDRESS="$gameserver_address" \
            FORTNITE_GAMESERVER_PLAYLIST="$gameserver_playlist" \
            FORTNITE_GAMESERVER_BUILD="$gameserver_build" \
            FORTNITE_GAMESERVER_PORT="$gameserver_port" \
            FORTNITE_GAMESERVER_BEACON_PORT="$gameserver_beacon_port" \
            FORTNITE_GAMESERVER_CAPACITY="$gameserver_capacity" \
            timeout --foreground --kill-after=5s "${seconds}s" bash -c '
                set -euo pipefail
                IFS= read -r FORTNITE_GAMESERVER_SECRET < "$1" || [[ -n "${FORTNITE_GAMESERVER_SECRET:-}" ]]
                printf -v cr '\''\r'\''
                printf -v lf '\''\n'\''
                case "$FORTNITE_GAMESERVER_SECRET" in
                    ""|*"$cr"*|*"$lf"*) printf "Invalid FORTNITE_GAMESERVER_SECRET_FILE contents.\n" >&2; exit 1 ;;
                esac
                export FORTNITE_GAMESERVER_SECRET
                exec bash -c "$2"
            ' _ "$gameserver_secret_file" "$FN_SERVER_CMD" &
    else
        setsid sudo -n -u "$user" env FORTNITE_NET_ENCRYPTION_KEY_FILE="${FORTNITE_NET_ENCRYPTION_KEY_FILE:-}" \
            timeout --foreground --kill-after=5s "${seconds}s" bash -c "$FN_SERVER_CMD" &
    fi
    server_cmd_pid=$!
    sleep "${FN_SERVER_START_WAIT:-5}"
fi
setsid sudo -n -u "$user" env "${wine_env[@]}" \
    "${display_cmd[@]}" timeout --foreground --kill-after=5s "${seconds}s" \
    bash -c '
cd "$0"
source "$1"
shift
start_client_actions
exec wine FortniteClient-Win64-Shipping.exe "$@"' "$bin_dir" "$porting_dir/tools/client_actions.sh" \
    "$FN_PORTAL_ARG" -nosound -NoSplash -log -epicapp=Fortnite -epicenv="$FN_EPICENV" \
    -epiclocale=en-us -skippatchcheck -nobe -noeac -fromfl=none \
    -AUTH_LOGIN=unused "-AUTH_PASSWORD=local-username:$username" -AUTH_TYPE=exchangecode "$@" \
    > "${FN_CLIENT_RUN_LOG:-$generated/original-client-run.out}" 2>&1 &
client_pid=$!
wait "$client_pid"
status=$?
set -e
printf 'Original client runner exit status: %s\n' "$status"
if [[ "$status" == 124 ]]; then
    exit 0 # A client still running at the observation deadline is expected.
fi
exit "$status"
