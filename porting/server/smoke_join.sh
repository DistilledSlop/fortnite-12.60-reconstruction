#!/usr/bin/env bash
set -euo pipefail

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
porting_dir=$(cd -- "$script_dir/.." && pwd)
stage_dir="$porting_dir/generated/linux-stage/LinuxNoEditor"
game_dir="$stage_dir/FortniteLinux"
server="$game_dir/Binaries/Linux/FortniteLinuxServer"
client="$game_dir/Binaries/Linux/FortniteLinux"
server_log="$porting_dir/generated/dedicated-server-client-test.log"
client_log="$porting_dir/generated/local-client-join-test.log"
map=${1:-/Game/Maps/FortniteEmptyDedicated}
server_extra=()
if [[ "$map" == /Game/Athena/Apollo/Maps/Apollo_Terrain* ]]; then
    server_extra=(-FortniteProbePawnGround)
fi

if [[ ! -x "$server" || ! -x "$client" ]]; then
    printf 'Stage the Linux client and FortniteLinuxServer binaries first.\n' >&2
    exit 2
fi

timeout --kill-after=5s 30s "$server" "$map" "${server_extra[@]}" \
    -nullrhi -nosound -unattended -port=7777 -log > "$server_log" 2>&1 &
server_pid=$!
trap 'kill "$server_pid" 2>/dev/null || true; wait "$server_pid" 2>/dev/null || true' EXIT

for attempt in {1..50}; do
    if rg -a -q 'IpNetDriver listening on port 7777' "$server_log"; then
        break
    fi
    sleep 0.1
done
if ! rg -a -q 'IpNetDriver listening on port 7777' "$server_log"; then
    printf 'Server did not listen on port 7777. See %s\n' "$server_log" >&2
    exit 1
fi

set +e
timeout --kill-after=5s 20s "$client" FortniteLinux 127.0.0.1:7777 -nullrhi -nosound -unattended -log > "$client_log" 2>&1
client_status=$?
set -e
if [[ $client_status -ne 0 && $client_status -ne 124 ]]; then
    printf 'Client exited with status %s. See %s\n' "$client_status" "$client_log" >&2
    exit 1
fi
if ! rg -a -q 'Join succeeded:' "$server_log" || ! rg -a -q 'Welcomed by server' "$client_log"; then
    printf 'Network join did not complete. See %s and %s\n' "$server_log" "$client_log" >&2
    exit 1
fi
if [[ "$map" == *'?game=/Script/FortniteGame.FortGameModeAthena'* ]]; then
    if ! rg -a -q 'FortPlayerPawn ready: .*authority=1' "$server_log" ||
       ! rg -a -q 'FortPlayerPawn ready: .*authority=0' "$client_log"; then
        printf 'Athena pawn did not spawn on both peers. See %s and %s\n' "$server_log" "$client_log" >&2
        exit 1
    fi
    if [[ "$map" == /Game/Athena/Apollo/Maps/Apollo_Terrain* ]]; then
        if ! rg -a -q 'Apollo terrain grid: 61/61 meshes spawned' "$server_log"; then
            printf 'Apollo terrain grid did not spawn. See %s\n' "$server_log" >&2
            exit 1
        fi
        server_tiles=$( { rg -a -o 'Apollo terrain tile [A-H][1-8] ready: authority=1 mesh=yes' "$server_log" || true; } | sort -u | wc -l)
        client_tiles=$( { rg -a -o 'Apollo terrain tile [A-H][1-8] ready: authority=0 mesh=yes' "$client_log" || true; } | sort -u | wc -l)
        if [[ $server_tiles -ne 61 || $client_tiles -ne 61 ]]; then
            printf 'Apollo terrain mesh count differs: server=%s client=%s (expected 61). See %s and %s\n' \
                "$server_tiles" "$client_tiles" "$server_log" "$client_log" >&2
            exit 1
        fi
        if [[ -f "$game_dir/Config/ApolloBackground.json" ]]; then
            if ! rg -a -q 'Apollo background: 27/27 placed meshes spawned' "$server_log"; then
                printf 'Apollo background placements did not spawn. See %s\n' "$server_log" >&2
                exit 1
            fi
            server_background=$( { rg -a 'Apollo background mesh .* ready: authority=1 mesh=yes' "$server_log" || true; } | wc -l)
            client_background=$( { rg -a 'Apollo background mesh .* ready: authority=0 mesh=yes' "$client_log" || true; } | wc -l)
            if [[ $server_background -ne 27 || $client_background -ne 27 ]]; then
                printf 'Apollo background mesh count differs: server=%s client=%s (expected 27). See %s and %s\n' \
                    "$server_background" "$client_background" "$server_log" "$client_log" >&2
                exit 1
            fi
        fi
        if ! rg -a -q 'Apollo pawn ground probe: grounded=1' "$server_log"; then
            printf 'Apollo network pawn did not settle on terrain. See %s\n' "$server_log" >&2
            exit 1
        fi
        if ! rg -a -q 'Apollo sun light ready: authority=0' "$client_log"; then
            printf 'Apollo light did not replicate to the client. See %s\n' "$client_log" >&2
            exit 1
        fi
    fi
fi

printf 'Local UE join succeeded. Logs: %s and %s\n' "$server_log" "$client_log"
