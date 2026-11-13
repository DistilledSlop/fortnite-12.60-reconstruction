# Server-first orchestration: handoff (2026-11-12, evening)

Written when the orchestrating session (Opus 5.5, Sonnet/Opus subagents) stopped for usage. Read with
`SERVER_FIRST_PLAN.md` and `R-HANDOFF.md`. **G11 is owned by another agent; do not work on it.**

## 1. What landed (compiled, not yet verified with the vanilla client)

All new code is active only with `-FortniteOriginalClient`. The server build of these sources is green (build log
`generated/builds/build-3.log`, binary built 2026-11-12 19:11 local, staged in `generated/linux-stage/...`).

| File | What | Origin |
|---|---|---|
| `native/.../Private/FortAthenaMatchFlow.cpp/.h` (1.7k lines) | G7 driver: playlist + `AthenaGameData`, spawned and filled `DefaultMapInfo_C`, flight path, storm centres, warmup (R1), ready counter (R4), aircraft phase + bus flight + `PlacePlayersInAircraft`/`EnterAircraft`, jump and forced exit, skydive with client-authoritative movement, safe-zone phases (R2), storm bits. Logs `MatchFlow: ...` | Opus subagent, from cards R1/R2/R4/R5/R6 |
| `native/.../Private/FortAthenaInventoryFlow.cpp/.h` | G8 slice 1: world-inventory grants (pickaxe from cosmetic loadout or default, Wall/Floor/Stair_W/RoofS, EditTool, Wood), stack/remove helpers, equip (follows `0x1423150b0`), `ServerExecuteInventoryItem` body. Logs `Inventory: ...` | Sonnet subagent |
| `FortniteAthenaGameplay.cpp` | Hooks for both files in `StartPlay`/`RestartPlayer`; test floor is now a scaled `PBWA_W1_Floor_C` building piece, centred from its bounds, top at z=-950; on non-Apollo maps the original-client path places the pawn on it | orchestrator |
| `tools/vanilla_play_run.sh` | `FN_ACTIONS`, `FN_SERVER_URL_OPTS`, `FN_SERVER_ARGS` env overrides | orchestrator |
| `tools/client_log_scan.py` (B2), `tools/stage_server_content.py` | Log summary with milestones; cooked-package import-closure stager | Sonnet subagents |

**Empty RPC bodies are bound at run time**, not spliced: `InstallRpcOverrides()` in both new files calls
`UFunction::SetNativeFunc` for `ServerExecuteInventoryItem`, `ServerAttemptAircraftJump` and
`ServerReadyToStartMatch`. No generator or promotion work was needed. When the bodies are later spliced through
`behavior/*.inc`, remove the rebinding (drafts are in `generated/agents/g7-behavior/`, `g8-behavior/`, which are ignored).

## 2. Runs this session (`generated/runs/20261112-play-run-N`)

- **28** (baseline, old floor code): same state as run 27; the floor was never created, because `RestartPlayer`
  returned before `PositionPawnOnTerrain` on non-Apollo maps (observed: no `Test floor` line, pawn Z -536000).
- **29** (floor fix + inventory): the floor was created, then the client **disconnected with NetChecksumMismatch on
  `StaticMeshActor.StaticMeshComponent0`** (server 771028908, client 2595112584; client.log line 9056). Our engine
  `UStaticMeshComponent` replicated layout differs from the client's. Never replicate engine components; the
  building-piece floor replaced it (not run yet). All item definitions failed to load because they were not staged.
- **30** (G7 + building floor + bulk staging): **the server crashed at startup** (`Assertion failed: DerivedType`,
  `MemoryImage.cpp:1064`, SIGSEGV) while it loaded newly staged cooked **material** packages
  (`/Game/Packages/Fortnite_UniqueMaterials_1/Builder_2`). The match-flow hard-import closure (3,515 packages,
  562 MB, through DefaultMapInfo, Athena_GameMode and SafeZoneIndicator) pulls in materials whose cooked shader-map
  memory images our 4.25 server cannot read. All 8,659 files staged this session were **moved** to
  `generated/agents/unstaged-20261112/Content/`. The stage is back to its pre-session state (848 MB).
  Run 30 says nothing about the G7 code.

## 3. Next steps, in order

1. **Staging without materials.** Add an exclude to `stage_server_content.py` (skip packages whose exports are
   Material/MaterialInstance*/MaterialFunction/Texture*/shader-bearing assets, or skip `/Game/Packages/`,
   `Materials/`, `Textures/` paths) and do not follow imports through them. Stage the **items** group and
   `PBWA_W1_Floor` first and confirm the server boots. Then stage the match-flow roots one at a time
   (`Playlist_DefaultSolo`, `Balance/DataTables/AthenaGameData`, `AthenaAircraft`, `GE_OutsideSafeZoneDamage`
   first; `DefaultMapInfo`, `SafeZoneIndicator`, `Athena_GameMode` last, because their closures are huge), booting
   the server after each one (`-nullrhi` start is enough, about 2 s to the crash point). Files to restore are in the
   quarantine folder.
2. **G6 close + G8 slice 1 run.** The command, about 15 min:
   ```
   FN_SERVER_URL_OPTS='?RequiredPlayers=1' FN_SERVER_ARGS='-RequiredPlayers=1 -FortniteDropZoneExtent=40000 -FortniteWarmupSeconds=20 -FortniteSafeZoneTimeScale=0.1 -FortniteStormCentered -FortniteFlightYaw=0' tools/vanilla_play_run.sh 900
   ```
   Then `python3 -I tools/client_log_scan.py <run>/client.log --server <run>/server.log` and the screenshots. Expect
   `Test floor ... bounds`, the pawn standing (tracker Z stable near -900), `Inventory: granted ...`,
   `Inventory: equipped ...`, and the `MatchFlow:` sequence listed in the G7 report (setup, warmup, aircraft, exit,
   safe zones). The client joins about 300 s after the start.
3. **Known G7 risks** (from the implementer): `FPlaylistPropertyArray::NetDeltaSerialize` may not replicate
   `BasePlaylist`, so watch for a G6 loading-screen regression. The server sends no `ClientEnterAircraft`. There is
   no server skydive physics (the client is authoritative while skydiving). Storm damage does nothing until health exists (next item).
4. **Stopped before writing anything, so restart them:** (a) G8 health/ability system (Opus): ASC on the
   PlayerState, attribute sets, the 31 `GAS_AthenaPlayer` abilities, `AFortPawn::HealthSet`, damage application,
   death hook into the D6 death sequence; new files `FortAthenaHealth.cpp/.h`. (b) G8 building (Sonnet):
   re-create `ServerCreateBuildingActor` (R4 §2, D5 §3), edit/repair/destroy; new files `FortAthenaBuilding.cpp/.h`.
   The building agent's last lead was `CachePlayerBuildableClasses` at `0x141e92db0` (decompile it for the
   build-class mapping). Brief template: `generated/agents/WORKER_RULES.md` (ignored, so copy it into the brief).
5. Then G8 pickup/drop/loot (D10), G9 end of match (R3, D6, E), G10 two clients.

## 4. Working rules that mattered

- One native writer: subagents only write their own new files; the orchestrator adds the hooks, builds, stages,
  runs and commits (`git commit --only -- <paths>`, Claude author + committer env, fast-forward push).
- Build: `Engine/Build/BatchFiles/Linux/Build.sh FortniteLinuxServer Linux Development -project=$PWD/native/FortniteLinux.uproject -NoHotReload`.
  Check the log for `error:`. The message `Expected FortniteAthenaGameplay.h to be first header included` is
  non-fatal and was there before this session. Copy the binary into the stage before a run.
- The UE log frame counter `[NNN]` wraps at 1000; it is not a stall indicator.
