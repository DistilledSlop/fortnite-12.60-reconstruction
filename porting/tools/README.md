# Runtime-image lookup tools

Small standard-library scripts for the decrypted client image
(`porting/generated/dump/FortniteClient-Win64-Shipping.runtime.exe`; file offsets equal RVAs). Run with `python3 -I`.

| Script | Use |
|---|---|
| `strx.py "text" ...` | Find ASCII/UTF-16 strings containing the text and the code that loads them: prints string address, reference address and the `.pdata` chunk start. `--secs` lists sections |
| `strs_near.py 0xA 0xB` | Dump UTF-16 strings in an address range (log strings of one source file sit together) |
| `vt.py 0xFUNC` | Find vtables that contain a function pointer and print the neighbouring slots |
| `nameidx.py out.json` | Build the name index: every function that loads a `Class::Function` log string. Output is `server_scope/string_names.json` |

Notes:
- `.pdata` splits large functions into chunks. A reported "function start" can be a chunk in the middle of a function; the real start is one of the few preceding chunk starts. Never measure function size from one `.pdata` entry.
- To decompile: `/opt/fortnite/local-model-package/scripts/evidence.py capture --input <json list of addresses inside the package dir>`; output in `captures/<hash>/<addr>.{c,asm,json}`.
- `server_scope/string_names.json`: address (chunk start) → up to four `[class, function, count]` guesses. 2,536 functions are named this way. A name comes from a log or ensure string inside the function, so it is strong evidence but can be the caller's name when a helper was inlined.

## Game data from the extracted content (added 2026-11-11)

Build the inspector once (the checked-in `bin/` is older than the source):

    dotnet build porting/cue4inspect/Cue4Inspect.csproj -c Release -o <scratch>/c4i

Then, for any asset under `/mnt/vibrato/fortnite-extracted-full`:

    dotnet <scratch>/c4i/Cue4Inspect.dll FortniteGame/Content/<path-no-ext> \
        /mnt/vibrato/fortnite-extracted-full --property-report --loose > out.txt 2>/dev/null
    python3 -I porting/tools/asset_props.py out.txt [maxwidth]      # tagged properties, one per line
    python3 -I porting/tools/asset_table.py out.txt [row-regex] [maxwidth]   # CurveTable / DataTable rows

`--property-report` now also prints the rows of curve tables and data tables
(field `table`). Curve rows print as `time:value` pairs.

## Which bodies are empty in the client (added 2026-11-11)

    python3 -I porting/tools/rpc_sweep.py Server > porting/server_scope/server_rpc_sweep.json
    python3 -I porting/tools/rpc_sweep.py ''     # every reflected native

Verdicts: `real`, `mixed` (trivial `_Validate` + real `_Implementation`, treat as real),
`inline` (the exec thunk is the body), `STUB` (empty, must be re-created),
`STUB_base_real_override` (empty in the declaring class, real in a subclass).
Results and designs: `porting/server_systems/D5-stubbed-rpcs-and-recreation.md`.

## Who calls this address (added 2026-11-11)

    python3 -I porting/tools/callers.py 0xADDR [0xADDR...]

Lists direct `call`/`jmp` references and data (vtable) references. No direct
callers plus a data reference means the function is a virtual. A server
function with no caller from the expected path means the calling code was
compiled out (example: `RemoveFromAlivePlayers`, see D6 card).

## Client/server log summary (added 2026-11-12)

    python3 -I porting/tools/client_log_scan.py <client.log> [--server <server.log>] [--json]

Groups Warning/Error/Fatal (and Display) lines of a UE log by category (LogNet, LogRep, LogFort*, LogAthena*, ...), with counts,
first/last timestamps, normalised example messages, a top-15 list and a milestones list (edit `CLIENT_MILESTONES` /
`SERVER_MILESTONES` at the top). Tokens, account ids, emails and auth strings are redacted. Output is about 70 lines.

## Staging server content (added 2026-11-12)

    python3 -I porting/tools/stage_server_content.py /Game/X/Y [...] [--dry-run] [--max N] [--soft] [-v]

Copies the cooked package(s) and the transitive hard-import closure (`/Game/...` packages from the .uasset import table,
parsed with the stdlib) from `/mnt/vibrato/fortnite-extracted-full/FortniteGame/Content` into
`generated/linux-stage/LinuxNoEditor/FortniteLinux/Content`. Never overwrites; prints visited/copied/missing, `/Script` modules and bytes.
`--soft` also follows `/Game/` names in the name map (soft refs). Default `--max` is 3000 packages; large roots (e.g. `Athena_GameMode`) need more.

## Second original client in a play run

Prepare its prefix once, **before** starting the first DXVK client:

```sh
porting/tools/second_client.sh --prepare
# Then, while vanilla_play_run.sh is active and its first client is in the frontend:
porting/tools/second_client.sh porting/generated/runs/<run> --username LocalPlayer2 \
  --renderer dxvk --actions '0:wait,UI.State.Startup.SubgameSelect,900 20:click,640,592 0:wait,ShouldShowBeginningOfSeasonDialog,600 25:click,1144,473 0:shot'
```

`FN_DXVK_PREFIX` selects the copy source (default `/tmp/fortnite-dxvk-experiment/prefix`).
The destination is `porting/generated/wine-prefix-2-dxvk`; copying a busy source is refused.
`--renderer builtin` selects Wine's D3D implementation. `--actions` defaults to `FN_ACTIONS`.
Both clients use `client_actions.sh`, including `wait`, `stopif`, and `shot`.
The helper uses the first client's namespaces/backend, a separate account and Xvfb,
and PID-scoped Curl/AppES/Oodle patches. It stops when that first client exits.
Logs are `<run>/client2/{runner.out,client.log}`; screenshots are `<run>/client2/shots/`.
Run it as the desktop user with passwordless sudo, just like the first runner.

To enable `FN_SECOND_CLIENT=1` automatically, make these additions to
`vanilla_play_run.sh` (these integration changes are intentionally shown here for the live-test owner).
Add `second_pid=` to the existing PID initialization; add this inside `cleanup()`:

```sh
    if [[ -n $second_pid ]]; then sudo -n kill -TERM "$second_pid" 2>/dev/null || true; wait "$second_pid" 2>/dev/null || true; fi
```

Insert immediately before `wait "$runner_pid"; status=$?; runner_pid=`:

```sh
if [[ ${FN_SECOND_CLIENT:-0} == 1 ]]; then
    (
        first_log="${FN_WINEPREFIX:-$porting/generated/wine-prefix}/drive_c/users/$USER/AppData/Local/FortniteGame/Saved/Logs/FortniteGame.log"
        while kill -0 "$runner_pid" 2>/dev/null; do
            if [[ $first_log -nt $out/start.marker ]] && grep -aq 'ShouldShowBeginningOfSeasonDialog' "$first_log"; then
                exec "$porting/tools/second_client.sh" "$out" \
                    --username "${FN_SECOND_USERNAME:-LocalPlayer2}" \
                    --renderer "${FN_SECOND_RENDERER:-${FN_RENDERER:-builtin}}" \
                    --actions "${FN_SECOND_ACTIONS:-$FN_ACTIONS}"
            fi
            sleep 1
        done
    ) > "$out/second-client-start.out" 2>&1 & second_pid=$!
fi
```

Immediately after that existing `wait` line, add:

```sh
if [[ -n $second_pid ]]; then wait "$second_pid" || echo 'Second client failed; see client2/runner.out and second-client-start.out'; second_pid=; fi
```

Use `FN_SERVER_URL_OPTS='?RequiredPlayers=2'` so the match waits for both players;
`FN_SERVER_ARGS='-RequiredPlayers=2'` also works and overrides the URL value.
The existing fallback matchmaking gives each ticket its own synthetic session ID,
but every session advertises `127.0.0.1:7777` and returns the runner's shared network
key. Independently pressing Play therefore targets the same native server; no
backend change is needed for that routing. A common backend session ID/roster
requires registered-server matchmaking. The two-client live join remains unverified.

## Live view of the running client (added 2026-11-13)

    nohup python3 -I porting/tools/live_view.py [--host 0.0.0.0] [--port 8766] \
        >> porting/generated/agents/live-view/server.log 2>&1 < /dev/null &
    pkill -f porting/tools/live_view.py      # stop

Open `http://<host>:8766/`. The page lists every running Xvfb display (found with `ps` on each request,
using its `-auth` file), gallery views at 1 fps / 640 px, and the clicked display at 24 fps (10 fps button).
Streams stop while the tab is hidden. `/stream/<N>?fps=&scale=` is a raw MJPEG stream of display `:N`;
`/api/displays` and `/api/captures` show what was found and which ffmpeg captures run.
One `ffmpeg -f x11grab` runs per (display, fps, scale) only while someone watches, shared by its viewers,
and stops about 3 s after the last viewer leaves. No frames are written to disk.
Port 8765 is the progress dashboard (`python3 -m http.server`), so the default here is 8766.
