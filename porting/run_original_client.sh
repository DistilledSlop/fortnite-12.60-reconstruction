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
# FN_SERVER_CMD starts a command (for example the dedicated server) in the same namespace.
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
seconds=${1:-120}
shift || true

if [[ -z "${FN_NETNS_INNER:-}" ]]; then
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
    exec sudo -n env FN_NETNS_INNER=1 FN_USER="$USER" FN_USERNAME="$username" FN_WINEPREFIX="$wineprefix" FN_WINEDLLOVERRIDES="${FN_WINEDLLOVERRIDES:-}" FN_VK_ICD_FILENAMES="${FN_VK_ICD_FILENAMES:-}" FN_LP_NUM_THREADS="${FN_LP_NUM_THREADS:-}" FN_PATCH_VERIFY="${FN_PATCH_VERIFY:-1}" FN_PATCH_APPES_DIAGNOSTIC="${FN_PATCH_APPES_DIAGNOSTIC:-0}" FN_ACTIONS="${FN_ACTIONS:-}" FN_ACTION_LOG="${FN_ACTION_LOG:-}" FN_SHOTS_DIR="${FN_SHOTS_DIR:-}" FORTNITE_NET_ENCRYPTION_KEY_FILE="${FORTNITE_NET_ENCRYPTION_KEY_FILE:-}" FN_STATE_DIR="${FN_STATE_DIR:-}" FN_SERVER_CMD="${FN_SERVER_CMD:-}" FN_WINEDEBUG="${FN_WINEDEBUG:--all}" FN_DISPLAY="${FN_DISPLAY:-}" FN_XAUTHORITY="${FN_XAUTHORITY:-}" FN_PORTAL_ARG="${FN_PORTAL_ARG:--NOEPICPORTAL}" FN_EPICENV="${FN_EPICENV:-Prod}" unshare -n -m "$porting_dir/run_original_client.sh" "$seconds" "$@"
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
client_log="$wineprefix/drive_c/users/$user/AppData/Local/FortniteGame/Saved/Logs/FortniteGame.log"
rm -f "$client_log" "$generated/server-requests.jsonl"
state_args=()
[[ -z "${FN_STATE_DIR:-}" ]] || state_args=(--state-dir "$FN_STATE_DIR")
python3 "$porting_dir/server/probe_server.py" --port 443 --stub "${state_args[@]}" \
    --tls-cert "$generated/tls/cert.pem" --tls-key "$generated/tls/key.pem" \
    --local-matchmaking-ws-port 0 \
    > "$generated/probe-server.out" 2>&1 &
server=$!
patcher=
appes_patcher=
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
oodle_patcher=
if [[ "${FN_PATCH_OODLE:-1}" == 1 ]]; then
    # The original client's Oodle handler would compress packets we cannot decode (no Oodle SDK); see the script header.
    python3 "$porting_dir/server/patch_oodle_min_size.py" "$seconds" > "$generated/patch-oodle-min-size.out" 2>&1 &
    oodle_patcher=$!
fi
trap 'kill $server $patcher $appes_patcher $oodle_patcher 2>/dev/null || true' EXIT

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
fi
# FN_SERVER_CMD: optional command (run as the user, inside this namespace) started before the
# client, e.g. the dedicated server. Stopped when the client run ends.
server_cmd_pid=
if [[ -n "${FN_SERVER_CMD:-}" ]]; then
    sudo -n -u "$user" env FORTNITE_NET_ENCRYPTION_KEY_FILE="${FORTNITE_NET_ENCRYPTION_KEY_FILE:-}" \
        timeout --kill-after=5s "${seconds}s" bash -c "$FN_SERVER_CMD" &
    server_cmd_pid=$!
    sleep 5
fi
sudo -n -u "$user" env "${wine_env[@]}" \
    "${display_cmd[@]}" timeout "${seconds}s" \
    bash -c '
cd "$0"
source "$1"
shift
start_client_actions
exec wine FortniteClient-Win64-Shipping.exe "$@"' "$bin_dir" "$porting_dir/tools/client_actions.sh" \
    "$FN_PORTAL_ARG" -nosound -NoSplash -log -epicapp=Fortnite -epicenv="$FN_EPICENV" \
    -epiclocale=en-us -skippatchcheck -nobe -noeac -fromfl=none \
    -AUTH_LOGIN=unused "-AUTH_PASSWORD=local-username:$username" -AUTH_TYPE=exchangecode "$@" \
    > "$generated/original-client-run.out" 2>&1
status=$?
set -e
[[ -z "$server_cmd_pid" ]] || { kill "$server_cmd_pid" 2>/dev/null || true; }
printf 'Original client runner exit status: %s\n' "$status"
if [[ "$status" == 124 ]]; then
    exit 0 # A client still running at the observation deadline is expected.
fi
exit "$status"
