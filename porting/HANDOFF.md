# Handoff: state of the Fortnite 12.60 Linux reconstruction

## Current snapshot (2026-11-01, after `95f1495`)

- **Native source `907e1dc`, closure tooling `2d99465`, engine `f716e0cb6`.** Native build and runtime suites pass separately: `generated/frontend-probes/build-getgameversion.log` and `generated/regress/20261101T002525Z-2242896/result.json` prove builds; `generated/regress/20261101T002339Z-2232871/result.json` proves restage, both joins, NetSerialize, 4,617/4,617 offline checksums, 251 behavior splices, 71 backend tests and schema checks. Generator checks pass 141. Earlier run `20261101T002007Z-2216551` has a wrapper FAIL solely because its old mtime rule rejected an already-current client despite Build.sh exiting 0. The external local-model wrapper now records both target hashes and accepts a valid incremental build. Do not hide the failed record or call it a compiler failure.
- **Actual UI dependency closure:** isolated `generated/frontend-probes/closure-fixed` visits 721 packages, 0 failures/0 pending, including Engine content after the corrected loose mount. `runtime-fixed-manifest.json` records the separate stage overlay without overwriting existing converted files. `UIManager_Startup_C` loads as BlueprintGeneratedClass (exit 0); RootLayout_Athena and LoginScreen still crash in original inline material shader-map parsing. The prior banner WidgetBlueprintGeneratedClass also loads with `-FortniteLegacyFieldPaths`; this does not establish widget construction/display or M1.
- **Material recovery is concrete but incomplete.** Primary implementation `8d3a3cb` reads the actual legacy library layout: version 2, 29-byte shader records and contiguous zlib code, unlike public UE 4.25's same-version six-array/LZ4 layout. The complete game index has 213,239 shaders; Global has 1,899. Four failing UI material files contain exact candidate hashes for 44 unique original DXBC shaders; all 44 are recovered. `53678a0` adds reproducible translation; `532d81a` flags diagnostics. All 44 translate and pass Vulkan 1.0 structural validation with both vkd3d 1.2 and locally built official Wine vkd3d 2.0; 12 emit unhandled modifier diagnostics with 1.2 and none with 2.0 (`f3e7403`). The descriptor set, binding and storage-class inventories are unchanged across versions; semantic equivalence is unproved. `df3986e` decodes the original D3D wrapper with exact prefix/DXBC/optional framing for all 44: 19 have active resource-table bits, and the maps contain 28 SRV, 18 sampler, 18 texture and zero UAV entries. `a02640f` inventories 180 translated SPIR-V descriptors across all 44, all in set 0. Register correspondence, UE Vulkan shader headers, uniform values and actual draws are not established. Reports/code/tools are ignored under `generated/material_pipeline/`; read its tracked README. Tests include seven reader cases, tampered-wrapper rejection, original/native archive reads and the 44-module run. A Luna read-only review independently checked the input/output hashes.
- **Next material work:** resolve compiler modifier behavior, decode the original inline shader-map and map D3D resources to the translated descriptors, bridge those shaders to UE's Vulkan resources, and test real draws against original output. GDB evidence `generated/frontend-probes/gdb-material-cursor-2.log` proves the alleged frozen-content size is ASCII `M_UI` (`0x49555f4d`), not a huge valid blob. `bd1c2e9` locates one original inline resource uniquely against the export map in each of the four failing material packages (bounds in `material_pipeline/README.md`). For Donut the resource spans `.uexp` offsets 2181..10601; the `M_UI` name bytes begin at 2225. A zero word immediately after the shader-map ID hash is read as `MaxFieldAlignment` by the local layout, then the original name length 16 is read as layout flags and `M_UI` as frozen size. This identifies the first schema divergence at the shader-map ID/base boundary, but the full original shader-map schema and Linux runtime bridge remain unresolved. The captured archive Tell 2020 and `.uexp` offsets use different coordinate systems; discard the earlier inferred `+205` origin. PowerToast names parent `M_UI_GradientPanel` as shader identity. Do not replace shaders/materials with invented approximations or skip the read as a parity solution. Original arithmetic graph nodes are stripped. No UI root has been displayed.
- **Other frontend prerequisites:** faithful dynamic GetGameVersion now runs and logs `v4.25 (0)` for this engine's build/branch/CL inputs; this is not the original client's version string. `22841af` records native helper probe chains for ExecuteClientEvent, GetFrontEndCamera and GetContext; unresolved registry, object and manager identities prevent faithful implementation. SetInputMode, SetFrontEndCamera and UpdateWatermark also remain open. Preserve evidence under generated/frontend-native-evidence and frontend-next-*. `95f1495` selects native FortGameInstance in config: an isolated stage loaded Entry, showed one live instance and completed a direct join; its UI methods remain largely stubbed. Recover authentic game-instance UI initialization after material/class loading; native defaults still Entry/Athena. No new helper was ported from these probes.
- **Content:** six directly imported frontend/lobby meshes now have hash-verified GLBs and original-path import settings (`057df9b`, selection and builder in asset_pipeline). Three exporter plus four manifest tests pass; UE import/cook, original materials/collision and runtime geometry are unverified. The frozen CUE tool snapshot under generated/asset_pipeline/tool-snapshot-frontend protects provenance. Coordinate shared CUE rebuilds before exports. Full extraction remains complete; most usable content, skeletal animation and audio remain open.
- **Backend/multiplayer:** profile-parser, session-key, websocket envelope/Play and ReadyCheck corrections plus party-roster session-key handling pass 72 backend tests (`365693f`). Party delimiter, admission, capacity and error status are local policies, not established Epic behavior; no reconstructed-client end-to-end lobby/matchmaking exists. Server-authority inventory and most gameplay remain unfinished. All four product milestones stay open; dashboard weighted effort remains about 24.10%, not gameplay completeness.
- **Ownership/preservation:** one native/engine/build/stage owner; original extraction is immutable and keys stay out of logs/Git. Preserve five pre-existing uncommitted engine files (gitdeps, three GLTF importer files, Linker) and the CUE4Parse static-mesh edit. Engine's opt-in compatibility branch is codex/fortnite-12.60-compat; never push it to Epic upstream. Main porting repo has no remote. No build/probe/download job remains running at this handoff.
- **Persistent agent configuration (user update 2026-11-01):** primary plans, orchestrates and reviews, and works directly when useful, including difficult implementation/debugging/problem solving. Use `gpt-6-luna` medium for small read-only tasks; `gpt-6.1-terra` medium or high for regular delegated work (high when complexity warrants); `gpt-6.1-sol` medium for really complex delegated tasks. Always `fork_turns: "none"`, at most three active subagents. Preserve this through compaction and all future sessions. This supersedes the earlier `gpt-6-sol` default. Global/workspace/project AGENTS and project memory contain these instructions.

Everything below is historical context. This snapshot supersedes earlier status,
counts, process claims and next-step instructions when they differ; verify before
following any older command.

Written when the session wound down (usage nearly out). Read `README.md` (goal), `master_plan.md` and
this file first. Everything below is what was true at the last commit on `main`.

## Third session update (2026-10-31, later; supersedes the numbers below where they differ)

Everything here is committed on `main`; the sections below are still correct for workflow and pitfalls.

- **Class parity is 4,572 of 4,572 offline** (`ec52422`; the generator emits net-service RPCs, const RPCs, namespaced enums and
  cross-module types). Build green, both smokes pass, network checksum 683914758 on both peers. `class_checksum.py` is the check.
- **Behaviour: 249 bodies spliced** (10th pass `589e1c1`; the 11th pass added the `struct_net` kind: five recovered
  `NetSerialize` bodies, `b6c483a`). A 39-case round-trip self-test passes (`351dffe`, run the binary with
  `-NetSerializeSelfTest`). `FAthenaPawnReplayData` still uses the marked fallback (its wire depends on runtime state and
  key-dependent calls). UE 4.25 rule: in `struct_net` snippets write `Ar << MyBool;`, there is no `SerializeBool` in non-editor builds.
  The 12th pass (`1fa3ef4`) ported nothing: every remaining target is blocked on unnamed members or helpers.
- **Vtable slot map** (`f2cecde`): `porting/behavior/build_vtable_slots.py` (5 s, writes the git-ignored `vtable_slots.json`) and
  `vtable_lookup.py <class> <hexoffset>`; 90% of slots validated against the dump. Game-declared virtuals have no names.
- **Full asset extraction is done**: all 31 paks, 963,427 distinct files, 88.3 GB, in `/mnt/vibrato/fortnite-extracted-full`
  (verified, duplicates audited; `porting/extraction_status.md`, `Cue4Inspect --extract-all`). The older closure tree is
  `porting/generated/extracted` (symlink to `/mnt/vibrato/fortnite-extracted`). No audio middleware files exist; audio is cooked
  SoundWave bulk (`.ubulk`). Raw extraction is its own dashboard leaf; conversion into usable UE content is barely started
  (77 of 4,719 static meshes, no skeletal meshes or animation).
- **Backend evidence** (`e8942b4`): `porting/server/EVIDENCE.md` and `endpoint_evidence.json` classify each served route
  (binary, wine_log, reflection, asset, convention). No real client capture can ever be obtained (Epic's servers are shut down), so
  the backend must follow what the client binary, the reflected structs and the paks require. 70 tests pass.
  **Open: the client's JSON response parser is not located.** Stock UE 4.25 writes camelCase keys but reads exact PascalCase names,
  so the shipped client uses its own parser; the exe has camelCase key text. Find it from the key-string load sites that
  `evidence_scan.py` lists, then confirm required keys and case sensitivity. 151 client routes in the exe are not served yet;
  `public_catalog` is 2 of 18 and the remaining responses have no struct evidence yet.
- **Server-side behaviour risk** (a design concern, not yet measured): the vanilla client sees only replicated results of server
  logic (storm, bus, damage, eliminations, match end, AI, loot). Much of that logic is in the client exe (server code is compiled
  in: `AFortGameModeAthena`, `Server*_Implementation`, authority branches) and the rest is data in the paks, but logic that exists
  only in Epic's dedicated-server binary would have no source, and timing and RNG cannot be checked without a reference server.
  An inventory (every replicated property and `Client*`/multicast RPC mapped to its server-side writer in the exe) is being built:
  see `porting/server_authority_inventory.md` once it lands.
- **Census** (`census_ui_audio.py`) is a gap-finder, not a prerequisite. Its 48-worker run over 108,701 packages keeps results in
  memory and writes only at the end (no checkpoint, no progress line); it can be paused with SIGSTOP/SIGCONT but a kill loses
  everything. `porting/native_class_coverage.md` (exe class list versus promoted set) replaces it for class selection.
- **Dashboard**: overall about 21.8%; leaf `assets.pak_extraction` added. Re-render with `python3 -I porting/progress/progress.py`
  after a commit (the post-commit hook renders asynchronously).
- **Process notes**: the Bash tool moves a foreground command to the background at about 180 s even with a 600000 ms timeout, so
  run UE builds in the background tee'd to a log in `porting/generated/` and wait with one `until ! kill -0 <pid>` loop. Stage
  specific paths when committing; the working tree usually holds other agents' in-flight edits.

### Wind-down state (end of the third session)

- `main` is clean at the last commit; no agent edits are in flight except the package builder (outside the repo). Class parity is
  4,591 of 4,591 (stage 1: the 19 FortniteUI interfaces, `443ef8e`). Class coverage analysis: `porting/native_class_coverage.md`
  (4,591 of 4,763 game classes generated; causes C1 to C5).
- **Next class step (stage 2) failed and was reverted**: SocialUMG and KairosSceneCapture promote but the UE build fails in
  UHT-generated code (a forward-declared FortniteGame class has no header or API macro in the generated `.cpp`). Cause and fix
  are in `porting/native_classes_status.md` ("Coverage stages"). The fix belongs in `gen_native_classes.py`; then retry batch 1
  (KairosSceneCapture goes in the three `*.Target.cs` `ExtraModuleNames`, not FortniteGame.Build.cs, to avoid a cycle), then the
  other 31 modules in batches of one to three; NVIDIAGfeSDK is skipped.
- **Server-authority inventory is PARTIAL** (`porting/server_authority_inventory.md`, `a0fbe1f`): the observable surface is
  complete (1,715 replicated properties, 734 net RPCs, 240 server-to-client); writer attribution is incomplete. Resume with the
  scripts and the saved `scan_c` data in `/opt/fortnite/local-model-package/inventory-wip/` (README there): rerun `analyze.py` and
  `picks.py` on `scan_c` first; the 1.1 GB `full.asm` is regenerated by the objdump step in its README.
- **Local-model package** (work queue, runbooks, scripts for a smaller model): `/opt/fortnite/local-model-package/` (outside the
  repo; start at its `README.md`). The model may commit; the rules there match this file's rules.
- **Census still running** (`python3 -I porting/census_ui_audio.py`, PID 3313931, started about 19:40, 48 Cue4Inspect workers, no
  checkpoint, writes `ui-audio-census.json` only at the end). It is low value (`native_class_coverage.md` replaced it for class
  selection); let it finish, or `kill -STOP 3313931` / `kill -CONT 3313931` to pause it. Killing it loses the run. It slows UE
  builds (about 2x).
- Dashboard: overall 24.07% at `1ca5d96` (denominators recounted from the full extraction; static meshes 77 of 4,727).
- Open questions, in order of value: (1) the client's JSON response parser (see `porting/server/EVIDENCE.md`); (2) the generator
  fix above and the 34 missing runtime modules; (3) server-authority writer attribution, then porting the match-flow behaviours
  (safe zone, game phase, drop zone, supply drops, bots, match end); (4) conversion of extracted assets into usable UE content
  (meshes, textures, skeletal meshes, audio) which is barely started.

## Where things stand

Updated 2026-10-31 at the end of the second session. Everything below is on `main` (HEAD `b9077d8` or later);
`main` is clean, no agent or build is running, and `/opt/fortnite/gen-worktree` is a leftover worktree on
`gen/helpers` (already merged; `git worktree remove /opt/fortnite/gen-worktree` when you no longer need it).

- **Dashboard:** about 20% overall (20.35% at `8c61c28`, 0 of 4 milestones; served on port 8765 when
  `progress/serve.sh` runs). The goal is far from met: no Fortnite gameplay runs, and function bodies are still
  mostly stubs (237 of 20,574 native functions ported). The evidence ledger holds the native join, checksum and
  self-test records recorded at `97d4db3`; re-record after big changes (see "Dashboard").
- **Verified at `eb27249` (build green in 246 s, both smoke variants pass, checksum parity unchanged):**
  - 4,463 classes promoted from the exe's reflection tables, class set of 4,572 (FortniteGame, FortniteUI,
    CommonInput, CommonUI, PlayspaceFramework, McpProfileSys, ModularGameplay, MeshNetwork, ...).
  - Offline class checksum parity with the original: 4,535 of 4,572 (37 remain: struct in another module 16,
    RPC flags without a role 13, unresolved members 5, enums 3).
  - Native client joins the native dedicated server with network checksum 683914758 and an Athena pawn
    (authority 1 on the server, 0 on the client).
  - Behaviour is spliced by the generator (`gen_native_classes.py`; `--no-behavior` or `FN_BEHAVIOR_SPLICE=0`
    turns it off): 237 entries of kinds getter, exec, onrep, impl, validate, helper, static_helper and
    member_field (16 non-reflected fields). Contract in `behavior/README.md`.
  - Function and delegate parameters are generated in binary offset order (`--param-order table` is the old
    order); 91 more delegate members emitted; replication conditions (`replication_conditions.json`, 1,659 of
    1,715 properties at high confidence) and `NetDeltaSerialize` traits for 43 fast-array structs.
- **Not runtime-tested:** the fast-array delta path, the conditional replication and the FText broadcast bodies
  compile and the join works, but no smoke exercises them. Nothing asserts that the C++ layout of the new
  non-reflected fields equals the binary offsets.
- **Known wire gaps:** six used structs with a custom non-delta `NetSerialize` (`SharedRepMovement`,
  `ReplicatedPhysicsPawnState`, `FortAISharedRepMovement`, `FortAIAttributeReplicationProxy`,
  `AthenaPawnReplayData`, `CreateBuildingActorData`) use a marked default-serialization fallback; their bodies
  are not recovered. Push-model flags are not emitted. 51 replication conditions are unknown (`COND_None` + TODO).
- **Backend:** `server/` has a tested local service layer (OAuth, accounts, MCP profile, party, presence,
  lightswitch, cloud storage; `python3 -I porting/server/test_services.py`, 61 tests). Each endpoint is labelled
  observed, inferred or guess in `server_surface.md`; no real client capture has driven them yet.
- **Extraction (AES keys in use):** the keys live in `/home/vibrato/.config/fortnite-port/aes_keys` (outside the
  repo, mode 600; built from `/opt/fortnite/12.60.md`; verified 31 of 31 paks mount). Agents may use them with
  `--key-file=<path>` and must never print, log or commit them. All extractions live in ONE tree,
  `porting/generated/extracted` (git-ignored). State when stopped: about 77,000 files, 8.8 GB, on `/`. The four
  Frontend/lobby entry closures are complete (0 pending); 4,680 packages recovered by the census; all 2,175
  registry WidgetBlueprints are extracted (`generated/widget-blueprints.txt`). The seeded soft-reference closure
  (`--save-closure --seed-list generated/widget-blueprints.txt --follow-soft`) was killed at about 77k files with
  a large frontier; re-running is safe (files are rewritten) and the queued-set edit in `Program.cs` makes the
  pending count unique. Census numbers in `ui_audio_census.md` are from before the soft closure (4,680 packages);
  `census_ui_audio.py` already has `widget_presence()` for the next run.

## How to resume

1. Check `git status` and `git log`; `main` should be clean. Rebuild only if you changed generator inputs:
   ```sh
   bash porting/promote_native_classes.sh      # regenerates native/Source/*/Generated (splice on)
   cd /opt/fortnite/UnrealEngine
   ulimit -c 0
   Engine/Build/BatchFiles/Linux/Build.sh FortniteLinux FortniteLinuxServer Linux Development \
     -project=/opt/fortnite/FortniteClient-12.60-CL-13477524/porting/native/FortniteLinux.uproject -NoHotReload
   ```
   Pass an explicit Bash tool timeout of 600000 ms (the default is 120 s and the build would move to the
   background). Restage by COPYING the two binaries into
   `porting/generated/linux-stage/LinuxNoEditor/FortniteLinux/Binaries/Linux` (no `BuildCookRun -archive`; it
   would clobber the stage). The staged client needs the CEF3 libraries hard-linked (`native/README.md`,
   "Staging the client"). A full build takes about 4-5 minutes.
2. Run BOTH smoke variants and require exit 0 (use `env -u LD_LIBRARY_PATH`, `stdbuf -oL -eL`, and a `rg`
   shim that execs `grep -E` on PATH; `rg` is not installed; a shim lives in the old session scratchpad):
   ```sh
   bash porting/server/smoke_join.sh
   bash porting/server/smoke_join.sh '/Game/Maps/FortniteEmptyDedicated?game=/Script/FortniteGame.FortGameModeAthena'
   ```
3. Run `python3 -I porting/class_checksum.py` and `python3 -I porting/behavior/check_behavior.py --gen-root
   porting/native/Source/FortniteGame/Generated --splice-report porting/generated/reflection/behavior_splice.json`.
   `check_behavior.py` does NOT compile-check: only a build catches a snippet error (the ninth pass found three
   UE 4.25 access problems that way: `UWorld::GameState` and `AActor::Role` are private, and the delegate
   spelling is `TMulticastDelegate<void, Args...>`).
4. Parallel work: generator edits run in a git worktree (`git worktree add /opt/fortnite/gen-worktree -b <branch> main`,
   symlink `porting/generated` and `porting/native/Plugins` read-only) while one agent owns `porting/native/` and
   the UE build. Merging a generator branch can conflict on `decompilation_notes.md` (every pass appends a
   section); keep both.

## Next steps, in order

1. **Finish extraction.** Move `porting/generated/extracted` to `/mnt/vibrato/fortnite-extracted` (871 GB free;
   `/` has about 70 GB and needs a 12-20 GB floor for builds): stop all Cue4Inspect processes, `rsync -a`, verify
   file count, bytes and a sample with `cmp`, rename the old dir, symlink the same path, test one small
   `--save-closure` through the symlink, then delete the old dir. Then run the seeded soft closure to frontier 0
   in the background with a log, re-run `python3 -I porting/census_ui_audio.py`, update `ui_audio_census.md`
   and use its ranked list of missing `/Script` classes (`FortSocialNameTextBlock`, `FortKismetLibrary` is
   hand-written, plugin classes for Paper2D, Niagara, GameplayAbilities) to extend `native_class_set.txt`.
2. **More behaviour.** Unported but unblocked-by-naming: `OnRep_Kills`, `OnRep_AIKills`, `OnRep_DeathInfo`
   (global Broadcast `0x140684de0` unidentified), `OnRep_TotalPlayerScore`, `OnRep_TeamKillScore`, and the
   PlayerController/PlayerState callers (each needs its own `static_helper` for `GetFortGameStateAthena`).
   Queue and identities: `behavior/CALLEE_PORT_QUEUE.md`, `decompilation_notes.md` (Eighth and Ninth pass).
3. **Recover the six fallback NetSerialize bodies** and the push-model flags; add a runtime test of replication
   conditions and fast arrays.
4. Remaining class blockers: 16 structs in other modules, 13 RPCs without a role, unresolved members.
5. Everything else on the master plan: Frontend/lobby UI from the extracted widget graph, audio playback,
   cooked Blueprint loading (field-path crashes), Apollo streamed levels, skeletal meshes and animation, and
   driving the backend with a real client capture.

## Still to do

- **Ghidra project and dump** live under `/opt/fortnite/ghidra-projects/fortnite-slim` and
  `porting/generated/dump/` (git-ignored). Do not re-run the full auto-analysis: it dies of heap
  exhaustion (Address Table analyzer). The slim analysis (about 25 minutes, 48 GB heap, address-table and
  decompiler analyzers off, `ghidra/SlimAnalysisOptions.java`) is the working configuration.

## Hard rules and decisions

- **Observed original-client access gate.** The Wine run stopped at `CheckEntitledToPlay`.
  Offline checks (`class_checksum.py`) and native client/server joins provide partial evidence;
  they do not establish original-client wire parity. The runtime dump carries no keys.
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
  and `wine_lobby_reached` have no recorded passing result.

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
