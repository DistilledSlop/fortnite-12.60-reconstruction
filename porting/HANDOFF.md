# Handoff: state of the Fortnite 12.60 Linux reconstruction

Written when the session wound down (usage nearly out). Read `README.md` (goal), `master_plan.md` and
this file first. Everything below is what was true at the last commit on `main`.

## Where things stand

Updated 2026-10-31 after the second session. Everything below is on `main` (HEAD `14757d9` or later).

- **Dashboard:** about 20% overall (the evidence ledger now holds the native join, the network checksum and the
  checksum self-test). The goal is far from met: no Fortnite gameplay runs, and function bodies are still
  mostly stubs (208 of 20,574 native functions ported).
- **Verified (build green, both smoke variants pass at `14757d9`):**
  - 4,463 classes promoted from the exe's reflection tables (FortniteGame, FortniteUI, CommonInput,
    CommonUI, PlayspaceFramework, McpProfileSys, ModularGameplay, MeshNetwork, ... ), class set of 4,572.
  - Offline class checksum parity with the original: 4,535 of 4,572 (37 remain: struct in another module 16,
    RPC flags without a role 13, unresolved members 5, enums 3, others).
  - Native client joins the native dedicated server with network checksum 683914758 and an Athena pawn
    (authority 1 on the server, 0 on the client).
  - Ported behaviour is spliced by the generator (`gen_native_classes.py`, `--no-behavior` or
    `FN_BEHAVIOR_SPLICE=0` turns it off): 208 bodies.
  - Function and delegate parameters are generated in binary offset order (`--param-order table` is the old
    order); 91 more delegate members emitted; replication conditions (`replication_conditions.json`, 1,659
    of 1,715 properties at high confidence) and `NetDeltaSerialize` traits for 43 fast-array structs.
- **Not runtime-tested:** the fast-array delta path, the conditional replication, and the FText broadcast bodies
  compile and the join works, but no smoke exercises them.
- **Known wire gaps:** six used structs with a custom non-delta `NetSerialize` (`SharedRepMovement`,
  `ReplicatedPhysicsPawnState`, `FortAISharedRepMovement`, `FortAIAttributeReplicationProxy`,
  `AthenaPawnReplayData`, `CreateBuildingActorData`) use a marked default-serialization fallback; their bodies
  are not recovered. Push-model flags are not emitted. 51 replication conditions are unknown (`COND_None` + TODO).
- **Backend:** `server/` has a tested local service layer (OAuth, accounts, MCP profile, party, presence,
  lightswitch, cloud storage; `python3 -I porting/server/test_services.py`). Each endpoint is labelled
  observed, inferred or guess in `server_surface.md`; none has been driven by a real client capture yet.

## How to resume

1. Check `git status` and `git log`; `main` should be clean. Rebuild only if you changed generator inputs:
   ```sh
   bash porting/promote_native_classes.sh      # regenerates native/Source/*/Generated (splice on)
   cd /opt/fortnite/UnrealEngine
   ulimit -c 0
   Engine/Build/BatchFiles/Linux/Build.sh FortniteLinux FortniteLinuxServer Linux Development \
     -project=/opt/fortnite/FortniteClient-12.60-CL-13477524/porting/native/FortniteLinux.uproject -NoHotReload
   ```
   Restage by COPYING the two binaries into `porting/generated/linux-stage/LinuxNoEditor/FortniteLinux/Binaries/Linux`
   (no `BuildCookRun -archive`; it would clobber the stage). The staged client needs the CEF3 libraries
   hard-linked (see `native/README.md`, "Staging the client"). A full build takes about 4-5 minutes.
2. Run BOTH smoke variants and require exit 0 (use `env -u LD_LIBRARY_PATH`, `stdbuf -oL -eL`, and a `rg`
   shim that execs `grep -E` on PATH; `rg` is not installed):
   ```sh
   bash porting/server/smoke_join.sh
   bash porting/server/smoke_join.sh '/Game/Maps/FortniteEmptyDedicated?game=/Script/FortniteGame.FortGameModeAthena'
   ```
3. Run `python3 -I porting/class_checksum.py` and `python3 -I porting/behavior/check_behavior.py --gen-root
   porting/native/Source/FortniteGame/Generated --splice-report porting/generated/reflection/behavior_splice.json`.
   `check_behavior.py` does NOT compile-check: only a build catches a snippet error.
4. Parallel work uses git worktrees for generator edits (`git worktree add /opt/fortnite/gen-worktree -b <branch> main`,
   symlink `porting/generated` and `porting/native/Plugins` read-only) while one agent owns `porting/native/`.

## Next steps, in order

1. **Splice kind for non-reflected helpers.** The generator splices only getters, exec and RPC kinds. Queue rows
   2, 3 and 5 (`behavior/CALLEE_PORT_QUEUE.md`) are non-reflected members of `AFortGameStateAthena`
   and friends; the 17 OnRep callers of `0x1419d3ff0` stay blocked until the generator can emit a declaration
   plus an `.inc` body for them. Also name the lock-counted 16-byte list family (`0x1407ac640`,
   `0x140685370`) and vfn slot `0x770` of `AFortPlayerStateAthena`.
2. **More behaviour.** Use `ghidra/probe_callees.py`, `ghidra/improve_decompile.py`,
   `ghidra/decompile_targets.py` and `decompilation_notes.md`.
3. **Recover the six fallback NetSerialize bodies** (see above) and the push-model flags.
4. Remaining class blockers: 16 structs in other modules, 13 RPCs without a role, unresolved members.
5. Everything else on the master plan: audio playback, Frontend/lobby UI, cooked Blueprint loading
   (field-path crashes), Apollo streamed levels, skeletal meshes and animation, a runtime test for replication
   conditions and fast arrays, driving the backend with a real client capture.

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

- The Bash tool default timeout is 120 s; pass `timeout: 600000` explicitly for UE builds or they move to the background.
- `check_behavior.py` does not compile; a snippet written in the old reversed delegate parameter order passes it
  but breaks the build. Read the generated header for the signature first.
- A `git merge` of a generator branch can conflict on `decompilation_notes.md` (every pass appends a section); keep both.
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
- **The evidence ledger holds the native join, checksum and self-test records** (recorded at `97d4db3`; `progress.py` reads
  the committed tree, so commit `progress/evidence.json` after re-recording). Re-record on an idle machine after
  a change that affects them:
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
