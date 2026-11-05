#!/usr/bin/env bash
set -euo pipefail

porting_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
stage_dir="$porting_dir/generated/linux-stage/LinuxNoEditor"
server="$stage_dir/FortniteLinux/Binaries/Linux/FortniteLinuxServer"
client="$stage_dir/FortniteLinux.sh"
map='/Game/Athena/Apollo/Maps/Apollo_Terrain?game=/Script/FortniteGame.FortGameModeAthena'
server_log="$porting_dir/generated/apollo-launch-server.log"

if [[ ! -x "$server" || ! -x "$client" ]]; then
    printf 'Linux client or server is missing from %s\n' "$stage_dir" >&2
    exit 2
fi
if [[ ! -f "$stage_dir/FortniteLinux/Config/ApolloLandscapeGrid.json" ]]; then
    printf 'Apollo terrain manifest is missing from the Linux stage.\n' >&2
    exit 2
fi

"$server" "$map" -FortniteLegacyFieldPaths -FortniteLegacyLandscapeData -nullrhi -nosound -unattended -port=7777 -log > "$server_log" 2>&1 &
server_pid=$!
cleanup() {
    kill "$server_pid" 2>/dev/null || true
    wait "$server_pid" 2>/dev/null || true
}
trap cleanup EXIT

ready=0
for ((attempt=0; attempt<100; attempt++)); do
    if rg -a -q 'IpNetDriver listening on port 7777' "$server_log"; then
        ready=1
        break
    fi
    if ! kill -0 "$server_pid" 2>/dev/null; then
        printf 'Apollo server exited during startup. See %s\n' "$server_log" >&2
        exit 1
    fi
    sleep 0.1
done
if (( ready == 0 )); then
    printf 'Apollo server did not listen within 10 seconds. See %s\n' "$server_log" >&2
    exit 1
fi

"$client" 127.0.0.1:7777 -FortniteLegacyFieldPaths -FortniteLegacyLandscapeData -vulkan -log "$@"
