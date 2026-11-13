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
