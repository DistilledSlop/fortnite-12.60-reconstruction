# Handoff: state of the Fortnite 12.60 Linux reconstruction

Written when the session wound down (usage nearly out). Read `README.md` (goal), `master_plan.md` and
this file first. Everything below is what was true at the last commit on `main`.

## Where things stand

- **Dashboard:** about 14.7% overall, 13.6% from computed measures, 0 of 4 milestones
  (`porting/progress/`, served on port 8765 if `progress/serve.sh` is running). The goal is far from met:
  no Fortnite gameplay runs, the original client cannot reach a handshake, and function bodies are
  almost all stubs (167 of 20,574 native functions ported).
- **Verified on `main`** (both smoke variants passed at the last green commit `8a38031`; later commits
  changed only docs, tools and the progress model):
  - 1,296 classes plus 109 interfaces generated from the exe's reflection tables across FortniteGame,
    PlayspaceFramework, McpProfileSys, CommonUI, ModularGameplay, MeshNetwork, BlueprintContext, Account,
    AmbientAudio; 1,107 structs, 462 enums, 581 delegates. Client, server and editor build.
  - Offline class checksum parity with the original: 1,317 of 1,405 generated classes.
  - Native client joins the native dedicated server with network checksum 683914758 and an Athena pawn.
- **Unverified, on branch `wip/native-fortniteui-unverified`** (one commit, 7,290 files): the native agent's
  next batch. Class set grown from 1,452 to 4,617 entries, new `FortniteUI` (about 1,680 classes) and
  `CommonInput` modules, Target.cs/Build.cs edits, refreshed generated trees. Its last build was killed
  mid-way and no smoke test was run on it. It may be fully green or may not compile.
- **Caution:** the built and staged binaries on disk (`native/Binaries`, `generated/linux-stage`) come from
  the WIP batch, not from `main`'s sources. Rebuild before trusting any smoke result on `main`.

## How to resume

1. Rebuild and verify `main` first (about 5 minutes), then try the WIP branch:
   ```sh
   cd /opt/fortnite/UnrealEngine
   ulimit -c 0
   Engine/Build/BatchFiles/Linux/Build.sh FortniteLinux FortniteLinuxServer Linux Development \
     -project=/opt/fortnite/FortniteClient-12.60-CL-13477524/porting/native/FortniteLinux.uproject -NoHotReload
   ```
   Restage by COPYING the two binaries into `porting/generated/linux-stage/LinuxNoEditor/FortniteLinux/Binaries/Linux`
   (no `BuildCookRun -archive`; it would clobber the stage). The staged client needs the CEF3 libraries
   hard-linked (see `native/README.md`, "Staging the client").
2. Run BOTH smoke variants and require exit 0 (use `env -u LD_LIBRARY_PATH`, `stdbuf -oL -eL`, and a `rg`
   shim that execs `grep -E` on PATH; `rg` is not installed):
   ```sh
   bash porting/server/smoke_join.sh
   bash porting/server/smoke_join.sh '/Game/Maps/FortniteEmptyDedicated?game=/Script/FortniteGame.FortGameModeAthena'
   ```
3. If the WIP branch fails to build, bisect by shrinking `porting/native_class_set.txt` (dependency-ordered,
   batches marked by `# Round` lines) and add unfixable classes to `porting/native_class_blocklist.txt`
   with a reason. Fix systematic failures in `porting/gen_native_classes.py`, never by hand-editing
   generated files. Merge the branch only after both smokes pass.
4. Record real evidence into the dashboard ledger on an idle machine (see "Dashboard" below).

## Next steps, in order

1. Verify/merge the WIP batch (above). Then run `python3 porting/class_checksum.py` and read
   `class_checksum_report.md` for the ranked fixes that flip the most classes to parity.
2. **Splice ported behaviour.** `porting/behavior/` holds 167 ported function bodies (snippets, see its
   README for the contract). The generator must (a) emit declarations for non-RPC functions and
   `_Validate`/`WithValidation`, (b) replace stub bodies with the `.inc` content, (c) add includes. Then
   build and smoke: the first behaviour-bearing build. `check_behavior.py` writes `behavior/COUNTS.txt`
   which the dashboard reads.
3. **More behaviour.** `behavior/BLOCKING_CALLEES.md` and `CALLEE_PORT_QUEUE.md` rank unidentified helpers
   (top blocker `0x1406a9dc0`, a pending-event dispatch blocking 18 functions). Use
   `ghidra/probe_callees.py`, `ghidra/improve_decompile.py`, `ghidra/decompile_targets.py` and
   `decompilation_notes.md` (method, helper identities, examples).
4. Remaining class blockers: 296 to 11 unresolved class refs (see `resolve_class_refs.py`), 9 structs with a
   custom `NetSerialize`, no `COND_*` replication conditions, ungenerated classes in modules still missing.
   Method that works for missing modules: generate them as their own minimal runtime module with the exact
   original name (as for PlayspaceFramework).
5. Everything else on the master plan: audio playback, Frontend/lobby UI, cooked Blueprint loading
   (field-path crashes), Apollo streamed levels, skeletal meshes and animation, the original-client path.

## Still to do

- **AES key commands:** three closure extractions listed in `master_plan.md` ("AES key commands")
  still need to run. They unlock the Frontend widget graph and the GUID-keyed paks. The AES keys
  must not be committed.
- **Ghidra project and dump** live under `/opt/fortnite/ghidra-projects/fortnite-slim` and
  `porting/generated/dump/` (git-ignored). Do not re-run the full auto-analysis: it dies of heap
  exhaustion (Address Table analyzer). The slim analysis (about 25 minutes, 48 GB heap, address-table and
  decompiler analyzers off, `ghidra/SlimAnalysisOptions.java`) is the working configuration.

## Hard rules and decisions

- **Anticheat bypass (EAC, BattlEye, AppES) still needs to be done.** The original client stops at
  `CheckEntitledToPlay`; `-nobe -noeac` are launch flags only. Wire parity is therefore verified offline
  (`class_checksum.py`) and with the native client against the native server, not with the original exe.
  The code used came from a read-only runtime dump that carries no keys.
- The supplied `*.id0/.id1/.nam` IDA files next to the exe are untrusted (unknown source). Not used.
- The exe's `.text` is encrypted on disk. Code was recovered from a read-only memory dump of our own Wine
  run (`dump_runtime_image.py`, `dump_to_pe.py`, `binary_analysis.md` section 6). Never write to that process.
- No keys or credentials in tracked files. Never force push. There is no git remote.
- Stage specific paths when committing. Subagents do not commit.
- Subagents: at most 3 at once, one owner for `porting/native/` and the UE build.

## Pitfalls that cost time

- IpNetDriver is **UDP**: check `ss -lun`, not `ss -ltn`. Redirected server stdout is block-buffered; use
  `stdbuf -oL -eL`.
- Never wait with `pgrep -f '<pattern that appears in your own command>'` or `pkill -f` with such a
  pattern: it matches itself, hangs or kills your own shell. Wait on a PID (`kill -0`) or a log line.
- Run UE builds in the foreground with the tool timeout set to 600000 ms, no `timeout` wrapper; a build
  longer than the tool limit is silently moved to the background and the agent "stops".
- `ulimit -c 0` before every build or test: crash tests leave multi-GB core files.
- Smoke script pawn check runs only in the Athena variant; run both variants.
- Wine runs need an idle machine: under UE-build load the client may not reach the Frontend in 90 s.

## Dashboard

- Model: `progress/components.json`; rendered by `progress/progress.py` (post-commit hook). Denominators in
  `progress/totals.json` (refresh with `python3 -I porting/progress/refresh_totals.py` after extraction
  changes). Measures: `checks`, `census`, `routes`, `tracked_count`, `tracked_regex`, `ledger`, `fraction`,
  `local_glob`; `README.md` has the objectivity audit and which leaves are still manual (each with a ceiling).
- **The evidence ledger is empty.** Leaves that read it (native join, Wine stage gates, EAC flags) show 0
  until recorded. Run on an idle machine and commit `progress/evidence.json`:
  ```sh
  python3 -I porting/progress/record_evidence.py smoke_default -- bash porting/server/smoke_join.sh
  python3 -I porting/progress/record_evidence.py smoke_athena -- bash porting/server/smoke_join.sh '/Game/Maps/FortniteEmptyDedicated?game=/Script/FortniteGame.FortGameModeAthena'
  ```
  The `wine_*` checks (`progress/README.md`, "Standard suite") need a run of
  `porting/run_original_direct.sh 240` first, then `--log` against the client log; `wine_entitlement_passed`
  and `wine_lobby_reached` are expected to fail.

## Storage

- `/opt/fortnite/FortniteClient-12.60-CL-13477524/Fortnite` is a symlink to `/mnt/vibrato/Fortnite` (the 86 GB
  original client; checksum-verified copy). UE source and build state stay on `/` (about 85 GB free).
- The Water plugin is Epic's 4.26 plugin adapted to 4.25; it is not in Git. Recreate it with
  `bash porting/prepare_water_plugin.sh`; regenerate its patch with `porting/regen_water_patch.sh`.

## Map of the tooling (all under `porting/`)

| Area | Files |
|---|---|
| Exe analysis | `binary_analysis.md`, `extract_reflection.py`, `extract_types.py`, `extract_native_funcs.py`, `resolve_class_parents.py`, `resolve_class_refs.py`, `class_checksum.py`, `validate_types.py` |
| Ghidra (PyGhidra, repo venv `porting/.venv`) | `ghidra/` (class names, type xrefs, decompile, improve, callee probe), `ghidra/name_tables/` |
| Generation | `gen_native_classes.py` (+ `.md`), `promote_native_classes.sh`, `native_class_set.txt`, `native_class_blocklist.txt`, `gen_stubs.py`, `native_classes_status.md` |
| Behaviour | `behavior/` (snippets, manifest, checker, bool bit decoder, blocking callees) |
| Backend | `server/` (mock Epic services, matchmaking, websocket, smoke tests) |
| Original client | `run_original_client.sh`, `run_original_direct.sh`, `direct_connect_results.md` |
| Progress | `progress/` |
