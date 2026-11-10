# Native recovery for the server (local_model)

Goal (2026-11-10): a full source body for every function in `queue.json` (1,110 functions that `server_scope` marks proven, strong or likely required and that are not ported). Claude hand-ports them; terra runs were stopped. **There is no "blocked" outcome.** An unnamed callee, member, virtual slot or timer is work to do (decompile it, name it, write it as a dependency), not a result. Do not add a blocked status to prompts, task files or reports.

## State (update this when batches land)

- Captured: all 1,098 unique exec addresses of the 1,110 are already decompiled into the package (`/opt/fortnite/local-model-package/captures/<hash>/<addr>.{c,asm,json}`); the list is `work/claude/addrs.json` (`Class::Function` -> address). Do not recapture.
- Terra pass: `tasks-br/0001` (match flow) produced 2 proposals (`HandleAllPlaylistLevelsVisible`, `GetGameplayMutator`) and 8 blocked records; those 8 must be redone as proposals. `tasks-br/` holds the task files, with 0001-0005 the BR-critical sets. `tasks/` is the older queue (kill path, creative tools, singles). The runner `terra_queue.py` is stopped.
- Hand decode already done: `JumpToSafeZonePhase` (body 0x141950090) and `SafeZoneInsideChecks` (0x1428f08d0, storm-camping timer at GameMode +0x14f0): `work/dependencies/claude-br/storm-and-phase-findings.json` in the package.
- Still needed for SafeZoneInsideChecks: decompile `0x14194cd50` (storm tick), `0x14232d430`, `0x142331080`, iterator `0x14192cca0`, pawn vfn `0x1010`.

## How to read a capture

Exec thunks are wrappers: `param_2+0x20 += (...)` is P_FINISH; results go through `param_3`; the body is inline, a direct `FUN_x(...)` (capture it with `evidence.py capture`, the request JSON must live inside the package dir), or a vtable dispatch `(**(code**)(*param_1+OFF))()` (resolve with `porting/virtual_slots.py` or `behavior/vtable_lookup.py`). Constant-return thunks are the cheap wins.

## Output per function

Body-only `.inc` (C++ statements, `// CONFIDENCE:` and original addresses), an `*.evidence.json`, then `inventory_progress.py record --owner claude-br --function Class::Fn --status proposed --evidence ... --inc ...`. Shared helpers, members and virtuals go under `dependencies/`. Order: const/getter thunks, direct-callee thunks, vtable ones, then the large match-flow functions.

## Haiku worker waves (2026-11-10)

Claude (orchestrator) runs Haiku subagents (Agent tool, `model: haiku`) over 74 chunks of 15 functions: `work/claude/chunks/NN.json` in the package, instructions in `work/claude/WORKER.md`, outputs in `work/claude/out/<Class>__<Fn>.{inc,evidence.json}` (+ `out/deps/`). Workers only write files; the orchestrator reviews and bulk-records proposals with `inventory_progress.py` (workers must not touch the state file) and commits. **At most TWO Haiku workers at a time (user rule).** Wave 1 = chunks 00-01 (pilot, check quality before launching more). Track which chunks are launched/reviewed/recorded here:

| Chunks | Status |
|---|---|
| 00 | done, reviewed (spot-checked HasHealthLeft against 0x141bad840), 15 recorded proposed |
| 01 | done, 15 recorded proposed (confidence mostly LOW/MEDIUM; worker reformatted 13 chunk-00 evidence JSONs, bodies unchanged) |
| 02 | done, 15 recorded proposed (some bodies LOW: unresolved receivers, partial helper reads, raw-offset comments to clean in a later review pass) |
| 03, 04 | done, 15 recorded proposed each |
| 05 | done, 15 recorded proposed |
| 06 | done, 15 recorded proposed |
| 07 | done, 15 recorded proposed |
| 08 | done, 15 recorded proposed |
| 09 | done, 15 recorded proposed (partial: big residual helpers) |
| 11 | done, 15 recorded proposed (GameState/Zone; several tag/rule helpers ported at API level only) |
| 10 | done, 15 recorded proposed (10 of 15 depend on partly-read large helpers) |
| 12 | done, 15 recorded proposed |
| 13 | done, 15 recorded proposed (several summaries, not full ports) |
| 14 | done, 15 recorded proposed (AFortPawn; thunks complete, big helpers PARTIAL) |
| 16 | done, 15 recorded proposed (AFortPlayerController; 5 depend on unported helpers) |
| 15, 17 | running (2 workers max at a time); chunks 09-55 use the regenerated BR-core-first order |

Recording a finished chunk: from the package dir `python3 -I work/claude/record_chunk.py NN` (claims or reopens each function as owner `claude-br`, then records proposed; copy in `claude/record_chunk.py`). Worker outputs are mirrored into `claude/` here (`<Class>__<Fn>.inc`, `.evidence.json`, `deps/`), with `claude/WORKER.md` the worker prompt. Chunk 00 helper deps include five not-yet-decompiled virtual targets (0x880, 0x888, 0x708, UpdateHighlightState, IsAthenaSpawnOverrideActive): chunk 00 follow-up item.

One-shot finish for a chunk (record + mirror + commit): from the package dir `work/claude/finish_chunk.sh NN`.

## Follow-up list (incomplete ports to redo; the goal needs whole bodies)

- `ABuildingSMActor::RepairBuilding` (chunk 04): repair visual branch and resource refund (callee 0x1417da1e0, 291 lines) not ported; vtable targets +0x10/+0x620 uncaptured.
- `ABuildingSMActor::MulticastPlayFullHealthEffects`, `ForceVisualState` (chunk 04): helper chains 0x141c1cd40 / 0x141c1a5f0 read only at purpose level.
- Chunk 00: virtual targets 0x880, 0x888, 0x708, UpdateHighlightState, IsAthenaSpawnOverrideActive undecompiled.
- Chunk 02: `BlueprintModifyIncomingDamage` receivers (virtual 0x1ae0, instigator controller) unresolved; large helpers 0x141befa00, 0x14409cc80, 0x1407babb0 read partially.
- Chunk 03: collision slots 0x800/0x810 names mismatched (AdjustCollisionOfStaticMesh, LockDevice), `MinigameStateChanged` slot 0xf08 (124 lines) partly traced.
- Chunk 05: `ABuildingTrap::TriggerIfAppropriate` and `AFortAIPawn::FastSharedReplication` LOW (helpers behind their deps unread, member names proposed); `SetCurrentGoalDiscouragement` parameter order inferred; StaticMeshComponent slot 0x4e0 target unknown (SetMirrored).

Scope note: the target is the 1,110 in queue.json only (user decision). The 3,057 unresolved and the 7,222 ceiling of porting/server_scope are out of scope for this goal.
- Chunk 06: `AFortAIPawn::NetMulticast_BatchedDamageCues` implementation 0x1422de590 only described in deps (port it); `PlayAdditiveHitReactsNative` montage helpers 0x143fd6e10/0x143fd6b30 read partially; `ReviveFromDBNO` LOW; `AbilitySpawnDeco` LOW (0x1426b5370 unexpanded); placeholder member names (AIFlags1344, ProtectedFlags520); capsule slot 0x800 target and predicate 0x1444aa6b0 unconfirmed.

## Scope cut (user decision, 2026-11-10: "cut the probably not ones")

280 functions in probably-not-BR classes (Creative, Minigame, Mission, Quest, MiniMap, Niagara/FX, Media, Analytics, MeshNetwork, Outpost, MangData, LockDevice, CaptureItem, Playground, Music, Ostrich/Meatball vehicles, etc.) are cut from the goal; the list is `claude/cut_not_br.json`. Chunks 00-08 (135 functions, already done or launched) are unchanged. Chunks 09-55 were regenerated from the remaining 695, BR-core classes first. New target = 135 + 695 = **830 functions** (was 1,110). Progress is N of 830.
- Chunk 07: helpers (0x1416a66e0 PlayEmoteInternal, 0x1416a83f0 HandlePerceptionStim, 0x1426f9960 water surface) kept as named deps not ported inline; receivers unresolved (emote tracker pawn+0x4c8, SwitchTeam game-state lookup, aileron-lock component +0x3b0, CheckWater FX/audio); trail-constant globals unnamed. `AFortAthenaCreativeSupplyDrop::SpawnLoot` is in a cut-class (creative) but already ported.
- Chunk 08: `AFortAthenaMutator_DadBro::StartNextSafeZonePhase` (storm phase logic, 1304-line decompile) has four helpers read by role only (0x141964450, 0x141936680, 0x141781400, 0x1410856e0); `PlayerTeamSettings::SetupGameRulesForTeamContext` LOW (slot 0xcd8 destructor-shaped); `RiftPortal` Teleport/NotifyTeleportedVehicle LOW (unidentified +0x800 object); `SKPushCannon` multicast FName global 0x148188be8 unresolved; proposed member names in deps/*_native_members.json.
- Chunk 09 (supply drop, vehicles, decotool): `AFortAthenaSupplyDrop::OnLootSpawned` ported as a no-op because the native body is behind a virtualized/unmappable stub (0x14191d000 -> 0x14998573a -> 0x14923284d): CHECK whether those addresses are real code in the dump or packed. Residual helpers to read/port: 0x1426d5640 (attach PostAttach virtual, ~6 KB), 0x1426b7e40 (deco slot 0x6e0, ~13.8 KB), 0x14266ee00 (6.7 KB), 0x142682ff0 (14.7 KB), 0x141a4cf10 (score events) and listener 0x1419a3380 vfn 0x938, engine bodies 0x143f72040/0x143f72550 (FlushNetDormancy/ForceNetUpdate), owner-class vfn 0x668. `SpawnPickupFromItemEntry` returns null (0x140680300 is xor/ret).
- Chunk 11 (GameState): helpers 0x1406a9dc0, 0x140744d30 (tag-count update, LOW), 0x1419b90a0/0x1419f3d90 (rule iterator, LOW), 0x14199a760 (delayed message path) not ported line by line; `HandleGameModeMessageRequest` LOW (struct partly recovered); `NotifyEndFailedGameplayVoteLockout` LOW; `IsStormCN` body folds to the same predicate as `IsLastManStandingCN` (name mismatch, verify).
- Chunk 10 (DecoTool, GameMode, GameState): large helpers not ported: DecoTool OnUnEquip cleanup (385 lines) and visibility helper (353), ServerCreateBuildingAndSpawnDeco building finder (112), ServerSpawnDeco CanPlaceDecoAt (583) + placement helpers (195, 254), TryToPlace return type guessed, SetJoinability session helpers, Client_RefreshEventCalendar refresh paths, start-loot-removal helper 0x14195e100. IMPORTANT CHECK: `FUN_140680300` decompiles as `return 0` and is used as the spawn call in `ServerCreateBuildingAndSpawnDeco` (chunk 10) and as the core of `AFortAthenaSupplyDrop::SpawnPickupFromItemEntry` (chunk 09). Confirm whether 0x140680300 is really a null stub or an ICF-folded/runtime-dump artifact (check callers and the dump bytes) before trusting those two bodies; if it is not a null stub, both are wrong. Unresolved event-name FName globals DAT_148192d60/d68/d70, DAT_1481af1a0, DAT_1481aef78.
- RESOLVED CHECK (0x140680300): it is genuinely `xor eax,eax; ret` (3 bytes, asm verified; 104 captured functions call it), a shared null/false stub, not a dump artifact. So `SpawnPickupFromItemEntry` and `ServerCreateBuildingAndSpawnDeco` spawn calls really do nothing in the CLIENT binary. This supports the earlier risk that some server logic is stripped from the client exe (lives only in Epic's dedicated-server binary): for any function whose body bottoms out in such a stub, the real server behaviour must be reimplemented from game semantics plus data, not recovered. Track these as `STRIPPED` in evidence notes.
- Chunk 12 (GameStateZone/mission): FIX `claude/AFortGameStateZone__ApplyPlayerAbilityRestriction.inc` (chunk 11): the byte at +0xf0 is `AActor::Role` (3 = ROLE_Authority), not an ability-restriction state; reflection names +0x8a8 `ExplicitGloballyBlockedAbilityTags`. `SetMissionRewardLootLevelOverride` / `SetMissionRewardOverrideTag` ignore their arguments in the native body (LOW; check Blueprint callers). `0x141eb2b40` (DeactivateGameplayModifierEntry, 7.7 KB) only partly read (PARTIAL). Engine helpers 0x1406a9dc0, 0x142cd3770, 0x1416b7770, 0x144640380 classified from call sites only.
- Chunk 13 (GE delivery actor, mutator, hook gun, pawn): LOW/summary only: `DoExplosion` (spec explode path), `DoSingleActorHit` (436-line impl summarised), `SetIgnoreInstigatorCollision` (ignore-list impl summarised), mutator state notifier helper; engine-level helpers (SetWorldLocation/SetWorldTransform/AttachToComponent, GetSocketByName, rope rotation math, FUN_144684d20) described not ported.
- Chunk 14 (AFortPawn): not ported: 0x1423150b0 (EquipWeapon impl, 241 lines), 0x142345360 (batched cue dispatcher, 379), 0x143fd6e10 (Montage_Play), 0x143fd56e0, 0x1446278d0 (SetTimer, 242), 0x1415e2c30, 0x1406b7f10, 0x140744d30 (205), 0x142314fe0. Unnamed virtual slots 0xc58, 0xa78, 0xae8, 0xaf0, 0xd08, 0x6b0, 0x6d0, 0x6d8, 0x810, 0x150. NetMulticast param lists are NOT reversed (others are). PickUpActor second param name looks wrong (used as WeaponData). 0x1406801d0 is Stub_ReturnTrue (not the null stub).
- Chunk 16 (AFortPlayerController): NOT PORTED/PARTIAL helpers (see out/deps/AFortPlayerController__Helper_*.json): 0x14237f580 (local-player helper, 280 lines), 0x141c74820, 0x141c7c5c0 (island lookup), callees of 0x14187f4e0 and 0x1423bd380, ServerCheatAll candidate chain; unresolved Blueprint-event FNames (ForceCancelBuildingTool, ModifyStat, OpenChoiceUI) and MCP JSON key constant (GetAccountLevel).
