#!/usr/bin/env bash
# Join a local Apollo dedicated server with a software-Vulkan client under a
# private Xvfb display and save one screenshot. Usage:
#   [CAPTURE_CLIENT_ARGS='-FortniteProbeLook=60,5'] capture_apollo_visual.sh <name> [port] [seconds]
# Writes generated/<name>.png and <name>.log (client) and <name>-server.log.
set -euo pipefail

porting_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
name=${1:?usage: capture_apollo_visual.sh <name> [port] [seconds]}
port=${2:-7791}
delay=${3:-28}
stage_dir="$porting_dir/generated/linux-stage/LinuxNoEditor"
server="$stage_dir/FortniteLinux/Binaries/Linux/FortniteLinuxServer"
client="$stage_dir/FortniteLinux.sh"
map='/Game/Athena/Apollo/Maps/Apollo_Terrain?game=/Script/FortniteGame.FortGameModeAthena'
server_log="$porting_dir/generated/$name-server.log"
client_log="$porting_dir/generated/$name.log"
png="$porting_dir/generated/$name.png"

if [[ ! -x "$server" || ! -x "$client" ]]; then
    printf 'Linux client or server is missing from %s\n' "$stage_dir" >&2
    exit 2
fi
rm -f "$png"

"$server" "$map" -FortniteLegacyFieldPaths -FortniteLegacyLandscapeData -nullrhi -nosound -unattended -port="$port" -log > "$server_log" 2>&1 &
server_pid=$!
trap 'kill "$server_pid" 2>/dev/null || true; wait "$server_pid" 2>/dev/null || true' EXIT
for ((attempt=0; attempt<150; attempt++)); do
    grep -a -q "IpNetDriver listening on port $port" "$server_log" && break
    kill -0 "$server_pid" 2>/dev/null || { printf 'Server exited. See %s\n' "$server_log" >&2; exit 1; }
    sleep 0.1
done

ulimit -c 0
export CAPTURE_PNG="$png" CAPTURE_DELAY="$delay" CAPTURE_CLIENT_ARGS="${CAPTURE_CLIENT_ARGS:-}"
xvfb-run -a -s '-screen 0 1280x720x24' bash -c '
    "$0" 127.0.0.1:"$1" -FortniteLegacyFieldPaths -FortniteLegacyLandscapeData -vulkan -ResX=1280 -ResY=720 -windowed -nosound -log $CAPTURE_CLIENT_ARGS > "$2" 2>&1 &
    client_pid=$!
    sleep "$CAPTURE_DELAY"
    ffmpeg -loglevel error -y -f x11grab -video_size 1280x720 -i "$DISPLAY" -frames:v 1 "$CAPTURE_PNG" || true
    kill "$client_pid" 2>/dev/null || true
    wait "$client_pid" 2>/dev/null || true
' "$client" "$port" "$client_log"

[[ -s "$png" ]] || { printf 'No screenshot written. See %s\n' "$client_log" >&2; exit 1; }
printf 'Captured %s\n' "$png"
