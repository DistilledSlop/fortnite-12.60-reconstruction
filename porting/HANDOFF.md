# Handoff: state of the Fortnite 12.60 Linux reconstruction

## Current snapshot (2026-11-01)

- **Goal:** feature-complete rebuilt Fortnite 12.60 client and reconstructed multiplayer server; all four dashboard milestones remain open. The revised **20.753%** estimates effort across 104 work items, with 1,855 total weight. Bounded material phases now receive ledger credit; the combined vertical-fill/unit-square treemap and historical measurements are preserved.
- **Verified source:** native `ebea44e`, Engine `6925baad3`. Final Menu client4/server2 builds and fresh default/Athena direct joins pass. All three owner probes and all 21 emitted shader payloads pass independent validation. Current exact commands, commit-blob source hashes, source/staged binary hashes and all 74 artifact/log hashes: `generated/material_pipeline/numeric-menu-checkpoint-results.json`; earlier Power checkpoint: `numeric-power-checkpoint-results.json`. Root independently checked those current binaries, committed source blobs and artifact hashes. No build/runtime process remains.
- **Four original owners now load:** Locker, GradientPanel, its PowerToastGlow instance and MenuButton preserve all 19 source records each, convert the seven Slate shaders their actual owners require, and attach on both threads with zero missing. PowerToast verifies five source parameter identities, eight graph/eight owner cases and a 3,952-pixel color change. The real parent is translucent (blend 2, flags 2816); its instance is opaque (blend 0, flags 2848), preserving the actual override. Six parent graph dependency packages were recovered verbatim.
- **MenuButton uses the real View buffer:** original metadata proves GameTime at byte 2264; native compiled metadata agrees. Current View is 4,032 bytes/61 resources/hash 130419238, while the original full struct is 3,952 bytes; only the observed GameTime access is proven compatible. Material is 304 bytes/hash 20447487. The original program changes 1,851 pixels between GameTime 0/3.25, remains identical when only RealTime changes, and changes all 4,096 pixels for a separate fixed-time Primary Color override. Eight numeric/four View cases pass. Artifacts/log: `generated/frontend-probes/legacy-numeric-menu-artifacts-4/` and `legacy-numeric-menu-4.log`; root validation receipt is beside the directory.
- **M1 remains unfinished:** no interactive root menu is displayed. Next implement Donut's original texture parameter/reference/sampler bindings, then rerun actual RootLayout/LoginScreen loading and construction to discover remaining dependencies and native lifecycle/helper behavior. Other materials/content, Editor rebuilds, lobby, matchmaking, full gameplay and original visual/wire parity remain open. The Menu result is not full View ABI or original-pixel parity.
- **Tools and evidence:** `6e59ff6` adds full CUE scalar/vector ParameterInfo plus immutable v2 numeric owner bundles and three inheritance tests; `71e44ff` captures the original View member chain and source-derived current prefix. The active Menu bundle is `generated/material_pipeline/menu-button-ingress-bundle-active-time.json`, pin `0b49a605763b907dbdf1f0041715208a81d4744a`. Earlier 0/1-second probes failed to exercise the original 3.0..4.5 shine window and remain retained; the unused quarter-time bundle is not the active pin. Verifier tests pass five, and typed View/Material bindings plus both layout hashes are independently checked.
- **Preserve:** one owner for native/Engine builds and staging; immutable extraction/external keys; five preexisting dirty Engine files plus the CUE static-mesh edit. Engine branch `codex/fortnite-12.60-compat`, never push to Epic; main has no remote. The portable ingress patch includes the committed Engine series through `6925baad3` and reverse-checks clean. Alternate STORE_ONLY_ACTIVE_SHADERMAPS reload has earlier syntax-only coverage, not runtime proof. Existing unrelated Water startup warnings remain; tested owner package dependencies have no missing imports.
- **Agent preference:** primary plans, orchestrates, reviews and also performs difficult work directly when useful. Luna medium for small read-only tasks; Terra medium/high for regular work; Sol medium for really complex work; always `fork_turns: "none"`, at most three subagents. Preserve through compaction and future sessions.

## Detailed evidence for this checkpoint

- **Latest numeric/View checkpoint:** final commands and immutable proof are in `generated/material_pipeline/numeric-menu-checkpoint-results.json` (native `ebea44e`, Engine `6925baad3`). Client4/server2 and both joins exit 0. Root verified committed source blobs, current source/staged binaries and all 74 recorded file hashes. Independent final verifiers: `legacy-numeric-menu-artifacts-4-root-validation.json`, `legacy-menu-final-power-root-validation.json`, `legacy-menu-final-locker-root-validation.json`. Menu readbacks prove two GameTime/RealTime isolation pairs and 1,851 changed pixels; fixed-time red/green changes 4,096. Native `25bf1c4`/Engine `6925baad3` is the separately retained Power checkpoint. Current ledger commit `769ff4f` adds Power/Menu credit and refreshes builds/joins; older receipt sources below are historical.
- **Next texture boundary:** Donut's serialized debug layout is 128 bytes with resource offsets 96/104/112/120 and types 5/8/8/8. The numeric-only factory and reader still reject its texture expression. Convert the actual source texture/reference/sampler and Vulkan resource-table associations, load the real texture and owner, and require actual-owner completeness and real sampling. Do not substitute an unrelated texture or white fallback. The full raw texture-expression grammar and original D3D resource-table associations are already decoded in the material reports.

- **Latest commands and bounded checks:** final client4 build exits 0 (110.23 s), server5 exits 0 (104.54 s), owner3 exits 0 (4.785 s), default/Athena join wrappers both exit 0. Exact commands and hashes are in the receipts above; owner command includes `-FortniteLegacyMaterialIngress=<absolute locker-ingress-bundle.json> -FortniteProbeLegacyOriginalMaterial -FortniteLegacyFieldPaths` on the isolated Vulkan stage. Three alternate-branch syntax commands/results are in `generated/material_pipeline/reload-store-only-syntax/results.json`; runtime reload is not proved. Invalid bundle rejection is recorded separately: probe-only exits 1, forced corrupt-package load sets archive error then ASL fatals (139); this is fail-closed rejection, not graceful recovery.
- **Dashboard evidence verification:** `python3 -I porting/material_pipeline/verify_progress_evidence.py all --report porting/generated/material_pipeline/dashboard-material-evidence.json` independently rechecks all four original maps (76 records/44 library shaders), all 44 mapped modules (180 descriptors/64 matched resource entries), four numeric layouts and the earlier eight-case detached draw. `python3 -I porting/material_pipeline/test_export_ingress_bundle.py` passes four tests; a corrupted original mesh record is rejected. `python3 -I porting/progress/test_history.py` passes four tests. `python3 -I porting/server/test_services.py` passes 72 tests (`generated/frontend-probes/dashboard-backend-tests-20261101.log`). The frontend import-manifest command in `asset_pipeline/README.md` verifies six source GLBs against frozen tool/input/output hashes; import, cook and runtime remain unverified.
- **Final artifact command:** `python3 -I porting/material_pipeline/verify_runtime_artifacts.py --fixture porting/generated/material_pipeline/locker-runtime-fixture.json --artifacts porting/generated/frontend-probes/legacy-material-ingress-artifacts-3 --report porting/generated/frontend-probes/legacy-material-ingress-artifacts-3-validation.json` passes all seven payloads/modules. Layout hash is 22544639; original mapped instructions differ only in entry-point names. Current owner readbacks retain the earlier red/green SHA-1 values and 2,832-pixel difference. Original-client pixel comparison remains unverified.

- **Earlier detached-rendering checkpoint:** native `afb32b7`, engine `757e0087a`. Both non-editor targets build; current default and Athena joins pass without diagnostic flags, with matching source/staged binary hashes (`generated/frontend-probes/runtime-material-joins-20261101T021940Z/`). Ledger `34deadf` records those joins/checksum and the detached original-material draw. The root independently validated all seven converted shader payloads and the two RGBA readbacks (`legacy-material-artifacts-2-validation.json`); PNG previews are beside the raw reads. The normal frontend still needs asset ingress/attachment, so all four milestones remain open.
- **Earlier native source `a494a4a`, closure tooling `2d99465`, engine `f716e0cb6`.** Its client and dedicated server builds exit 0 (`generated/frontend-probes/build-original-vulkan-client-guard.log` and `build-original-vulkan-server-guard.log`). Default and Athena smoke joins with freshly built binaries exit 0 without the probe flag in an isolated matched stage; Athena reaches pawn-ready on both peers (`generated/frontend-probes/original-vulkan-joins/`, including results, logs, exit statuses and binary hashes). The broader historical `907e1dc` build and runtime suites pass separately: `generated/frontend-probes/build-getgameversion.log` and `generated/regress/20261101T002525Z-2242896/result.json` prove builds; `generated/regress/20261101T002339Z-2232871/result.json` proves restage, both joins, NetSerialize, 4,617/4,617 offline checksums, 251 behavior splices, 71 backend tests and schema checks. Generator checks pass 141. Earlier run `20261101T002007Z-2216551` has a wrapper FAIL solely because its old mtime rule rejected an already-current client despite Build.sh exiting 0. The external local-model wrapper now records both target hashes and accepts a valid incremental build. Do not hide the failed record or call it a compiler failure.
- **Actual UI dependency closure:** isolated `generated/frontend-probes/closure-fixed` visits 721 packages, 0 failures/0 pending, including Engine content after the corrected loose mount. `runtime-fixed-manifest.json` records the separate stage overlay without overwriting existing converted files. `UIManager_Startup_C` loads as BlueprintGeneratedClass (exit 0); RootLayout_Athena and LoginScreen still crash in original inline material shader-map parsing. The prior banner WidgetBlueprintGeneratedClass also loads with `-FortniteLegacyFieldPaths`; this does not establish widget construction/display or M1.
- **Material recovery is concrete but incomplete.** Primary implementation `8d3a3cb` reads the actual legacy library layout: version 2, 29-byte shader records and contiguous zlib code, unlike public UE 4.25's same-version six-array/LZ4 layout. The complete game index has 213,239 shaders; Global has 1,899. Four failing UI material files contain exact candidate hashes for 44 unique original DXBC shaders; all 44 are recovered. `53678a0` adds reproducible translation; `532d81a` flags diagnostics. All 44 translate and pass Vulkan 1.0 structural validation with both vkd3d 1.2 and locally built official Wine vkd3d 2.0; 12 emit unhandled modifier diagnostics with 1.2 and none with 2.0 (`f3e7403`). The descriptor set, binding and storage-class inventories are unchanged across versions; semantic equivalence is unproved. `df3986e` decodes the original D3D wrapper with exact prefix/DXBC/optional framing for all 44: 19 have active resource-table bits, and the maps contain 28 SRV, 18 sampler, 18 texture and zero UAV entries. `a02640f` inventories 180 translated SPIR-V descriptors across all 44, all in set 0. `c608eab` uses vkd3d scan and explicit interface mappings: the mapped receipt has 44 validated shaders, 180 descriptors, all 64 original resource-table entries matched, zero unmatched and zero compiler diagnostics. It records descriptor-set and binding literal word offsets for UE remapping; an independent audit checked hashes, offsets and counts. Shader 25 at binding base 32 changes only five Binding literal words. The detached Locker bridge now exercises headers and live numeric values as described below; the later Locker quad draw is described below; normal attachment remains unresolved. Reports/code/tools are ignored under `generated/material_pipeline/`; read its tracked README. Tests include seven reader cases, tampered-wrapper rejection, original/native archive reads and the 44-module run. A Luna read-only review independently checked the input/output hashes.
- **Material constants and Vulkan factory:** `e78b1ab` reflects actual mapped modules: 100 uniform buffers, 28 uniform texel buffers, 26 sampled images and 26 samplers, with declared spans and stage interfaces. `9ef42e1` and `9e737fb` evaluate serialized defaults: 96/320/288/192 bytes for Donut/Locker/MenuButton/PowerToast, matching their debug first-resource offsets; six tests pass. `556e382` and `848489c` link 76 parsed Slate and mesh records and 44 Material uniform-buffer bindings through original serialized slots to explicit descriptors and evaluated bytes; all declared spans fit, and three tests pass (`generated/material_pipeline/material-constant-bindings.json`). Floating-point, instance inheritance and resource values remain limited. `0710dfd` builds the client and tests original geometry shaders 26 and 2459 in isolated software Vulkan: real `FVulkanShaderHeader` round trip and RHI factory pass and modified SPIR-V validates, but no Vulkan module, pipeline or draw is created. The successful `generated/frontend-probes/original-vulkan-probe-overlay.log` uses an overlay of 330 cooked Engine files and two matching archives documented in `original-vulkan-stage/overlay-manifest.json`; preserve the three earlier exit-139 graphics-startup logs from stale baseline material/archive files.
- **Next material work:** validate compiler behavior and semantics, reconstruct the decoded shader maps at runtime, bridge those shaders to UE's Vulkan resources, and test real draws against original output. GDB evidence `generated/frontend-probes/gdb-material-cursor-2.log` proves the alleged frozen-content size is ASCII `M_UI` (`0x49555f4d`), not a huge valid blob. `bd1c2e9` locates one original inline resource uniquely against the export map in each of the four failing material packages (bounds in `material_pipeline/README.md`). For Donut the resource spans `.uexp` offsets 2181..10601; the `M_UI` name bytes begin at 2225. A zero word immediately after the shader-map ID hash is read as `MaxFieldAlignment` by the local layout, then the original name length 16 is read as layout flags and `M_UI` as frozen size. The strict parser now decodes all four original serialized shader maps to exact EOF, including packed flags and 19 sequential records per material (seven Slate, 12 LocalVF), zero pipelines and cooking marker 1. All 76 records have archive hash, frequency and uncompressed size validated; 17 isolated tests pass. The texture record is `int32`, `int16`, `int16`, `int32`, `bool32` (16 bytes, as in 4.24); discard the earlier 4.23/five-word comparison and first-unknown offsets. The reports mark complete serialized maps but explicitly do not verify runtime loading. The captured archive Tell 2020 and `.uexp` offsets use different coordinate systems; discard the earlier inferred `+205` origin. PowerToast names parent `M_UI_GradientPanel` as shader identity. Do not replace shaders/materials with invented approximations or skip the read as a parity solution. Original arithmetic graph nodes are stripped. No UI root has been displayed.
- **Other frontend prerequisites:** faithful dynamic GetGameVersion now runs and logs `v4.25 (0)` for this engine's build/branch/CL inputs; the original client log reports `++Fortnite+Release-12.60-CL-13315662` (engine and compatible CL 13315662), despite the workspace folder name `CL-13477524`; network CL is separately 12896485. Reproducing its version string needs project-specific build metadata, rebuild and runtime verification. `22841af` records native helper probe chains for ExecuteClientEvent, GetFrontEndCamera and GetContext; new ignored `frontend-native-evidence` event-registry, event-registration and event-registration-lifecycle evidence identifies GameInstance class `0x1428ea7f0`, FortClientEventInterface `0x142877b10`, and the weak listener registry at `+0x6c0` with registration and removal. The fallback is `HandleClientEvent_Undefined`; lifecycle ownership and argument behavior remain unknown, as do other object and manager identities, preventing faithful implementation. SetInputMode, SetFrontEndCamera and UpdateWatermark also remain open. Preserve evidence under generated/frontend-native-evidence and frontend-next-*. `95f1495` selects native FortGameInstance in config: an isolated stage loaded Entry, showed one live instance and completed a direct join; its UI methods remain largely stubbed. Recover authentic game-instance UI initialization after material/class loading; native defaults still Entry/Athena. No new helper was ported from these probes.
- **Detached Locker runtime bridge:** Engine `757e0087a` implements neutral IR/API, live current preshaders, seven registered Slate constructors and checked known-output-hash finalization with normal freeze/resource initialization; main `4a95007` contains the portable `patches/ue425-legacy-slate-material-bridge.patch` (reverse check passed). Native `2144eba` accepts `-FortniteLegacyMaterialFixture=<.../locker-runtime-fixture.json>` and `-FortniteLegacyMaterialArtifactDir=<fresh existing empty dir>`. The fixture (SHA-1 `3c379d3b0b97bb8e57e9b7b8b7286182ac78a700`) has 220 nodes, seven records and eight cases; 312 meaningful numeric bytes are checked within a 336-byte layout. Non-editor client build passed (821 actions, 403.7 s; incremental 126.7 s); the dedicated server build passed in 436.1 s against Engine `757e0087a` and native `2144eba`. Actual software Vulkan/Xvfb run exited 0, passed eight live `FillUniformBuffer` cases, froze the detached map and constructed seven RHI shaders (`generated/frontend-probes/legacy-material-runtime-1.log`). Independent verification of 14 artifacts and seven actual SPIR-V/payloads passed: instructions unchanged except entry name, header offsets match descriptors, current layout hash 22544639 derives from original shape, and SourceHash/CRC/embedded code match (`generated/frontend-probes/legacy-material-artifacts-1-validation.json`; three tests). Native `afb32b7` adds the opt-in draw; its incremental server build passed in 117.39 s (`generated/material_pipeline/build-runtime-material-server-2.log`), and its actual Vulkan run exited 0: eight pre-freeze and eight post-freeze uniform cases passed, and a 64×64 Locker quad readback changed 2,832 of 4,096 pixels between two `RBGA1` overrides; center pixels were red `(255,0,0,255)` and green `(0,255,0,255)` (`generated/frontend-probes/legacy-material-runtime-draw-1.log`, `legacy-material-artifacts-2/LockerQuad.json` and associated RGBA files). Comparison against original-client rendered output remains unverified. Next prove normal material attachment, required mesh shaders and visual parity. See `material_pipeline/UE_BRIDGE.md`.
- **Content:** six directly imported frontend/lobby meshes now have hash-verified GLBs and original-path import settings (`057df9b`, selection and builder in asset_pipeline). Three exporter plus four manifest tests pass; UE import/cook, original materials/collision and runtime geometry are unverified. The frozen CUE tool snapshot under generated/asset_pipeline/tool-snapshot-frontend protects provenance. Coordinate shared CUE rebuilds before exports. Full extraction remains complete; most usable content, skeletal animation and audio remain open.
- **Backend/multiplayer:** profile-parser, session-key, websocket envelope/Play and ReadyCheck corrections plus party-roster session-key handling pass 72 backend tests (`365693f`). Party delimiter, admission, capacity and error status are local policies, not established Epic behavior; no reconstructed-client end-to-end lobby/matchmaking exists. Server-authority inventory and most gameplay remain unfinished. All four product milestones stay open; dashboard weighted effort remains about 24.10%, not gameplay completeness.
- **Ownership/preservation:** one native/engine/build/stage owner; original extraction is immutable and keys stay out of logs/Git. Preserve five pre-existing uncommitted engine files (gitdeps, three GLTF importer files, Linker) and the CUE4Parse static-mesh edit. Engine's opt-in compatibility branch is codex/fortnite-12.60-compat; never push it to Epic upstream. Main porting repo has no remote. Check for active jobs before starting shared builds or probes.
- **Dashboard refresh:** the post-commit hook now finishes rendering before returning. Normal renders repair missing reachable commit points without replacing historical measurements; use `--seed` to recompute every point. The current smoke/checksum ledger judges the final isolated join logs with wrapper exits and independently checked binary-hash provenance (`34deadf`); `cf75423` is the earlier baseline. Rewritten Vulkan modules also have a root-reviewed receipt at `generated/frontend-probes/original-vulkan-rewrite-validation.json`.
- **Next M1 implementation:** `material_pipeline/UE_MATERIAL_INGRESS.md` (`b0597c2`) identifies the bounded, opt-in provider before the current shader-map deserializer, complete original-record preservation, explicit selected-resource bounds, and actual-owner `IsComplete` validation before normal game-thread attachment. Current non-editor MD_UI LocalVF rules may exclude the original mesh extras, but only the real owner's required layout can establish this. Async deserialization must not call the diagnostic render-command flush. Other layouts, Donut textures, View buffers, compilation flags and Editor identity remain required work.
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
  after a commit (the current hook finishes rendering before returning).
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
