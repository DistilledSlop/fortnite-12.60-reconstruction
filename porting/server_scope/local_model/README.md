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
| 06, 07 | running (2 workers max at a time) |

Recording a finished chunk: from the package dir `python3 -I work/claude/record_chunk.py NN` (claims or reopens each function as owner `claude-br`, then records proposed; copy in `claude/record_chunk.py`). Worker outputs are mirrored into `claude/` here (`<Class>__<Fn>.inc`, `.evidence.json`, `deps/`), with `claude/WORKER.md` the worker prompt. Chunk 00 helper deps include five not-yet-decompiled virtual targets (0x880, 0x888, 0x708, UpdateHighlightState, IsAthenaSpawnOverrideActive): chunk 00 follow-up item.

One-shot finish for a chunk (record + mirror + commit): from the package dir `work/claude/finish_chunk.sh NN`.

## Follow-up list (incomplete ports to redo; the goal needs whole bodies)

- `ABuildingSMActor::RepairBuilding` (chunk 04): repair visual branch and resource refund (callee 0x1417da1e0, 291 lines) not ported; vtable targets +0x10/+0x620 uncaptured.
- `ABuildingSMActor::MulticastPlayFullHealthEffects`, `ForceVisualState` (chunk 04): helper chains 0x141c1cd40 / 0x141c1a5f0 read only at purpose level.
- Chunk 00: virtual targets 0x880, 0x888, 0x708, UpdateHighlightState, IsAthenaSpawnOverrideActive undecompiled.
- Chunk 02: `BlueprintModifyIncomingDamage` receivers (virtual 0x1ae0, instigator controller) unresolved; large helpers 0x141befa00, 0x14409cc80, 0x1407babb0 read partially.
- Chunk 03: collision slots 0x800/0x810 names mismatched (AdjustCollisionOfStaticMesh, LockDevice), `MinigameStateChanged` slot 0xf08 (124 lines) partly traced.
- Chunk 05: `ABuildingTrap::TriggerIfAppropriate` and `AFortAIPawn::FastSharedReplication` LOW (helpers behind their deps unread, member names proposed); `SetCurrentGoalDiscouragement` parameter order inferred; StaticMeshComponent slot 0x4e0 target unknown (SetMirrored).
