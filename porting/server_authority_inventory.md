# Server authority inventory (PARTIAL)

**Status: PARTIAL.** The observable surface is complete. The writer search is partial: it covers only property stores, and it was run before a known thunk-attribution bug was fixed. RPC send stubs were not located. Nothing here is a port. This document is analysis only.

Machine-readable output: `porting/generated/server_authority_inventory.json` (git-ignored, 580 KB, from the scan_b run). Scripts and a README are in `/opt/fortnite/local-model-package/inventory-wip/`.

## 1. Observable surface (complete)

Source: `porting/replication_conditions.json` (the file is at the porting root, not under `generated/reflection/`), `porting/generated/reflection/class_checksums.json` (native class set, 4,572 classes, with parent and module), `porting/generated/reflection/fortnite_game_reflection.json` (per-class properties with offsets, rep_notify, and RPC net_roles, joined by block_va), and `porting/generated/reflection/native_funcs.json` (exec addresses). The headers under `porting/native/Source/FortniteGame/Generated/` agree: 1,705 `Replicated` UPROPERTY specifiers in the generated tree, and the native set gives 1,715 replicated properties.

- Replicated properties: **1,715** in 325 native classes (319 in FortniteGame, 4 PlayspaceFramework, 1 AmbientAudio, 1 MeshNetwork). Conditions: COND_None 1468, OwnerOnly 99, InitialOnly 48, SkipOwner 40, Custom 27, SimulatedOnly 22, ReplayOnly 9, SkipReplay 1, InitialOrOwner 1. 878 have a RepNotify.
- RPCs by net role (734 net functions): Server 353, Client 158, Multicast 82, Reliable with no role 117, Response 24. **Observable server-to-client RPCs = Client 158 + Multicast 82 = 240.** The 117 no-role and 24 Response entries are not classed as observables here; their direction is unknown.
- Note: `replication_conditions.json` reports 1,961 properties over 370 classes. That total includes engine classes outside the native set. The 1,715 figure is the native-set count used above.

## 2. Method (what was run)

1. Full disassembly of the runtime dump `porting/generated/dump/FortniteClient-Win64-Shipping.runtime.exe` with `objdump -d -M intel --no-show-raw-insn` (30.3 M instructions, both code sections). Function bounds come from the `.pdata` table (478,657 functions).
2. Streaming scan (`scan.py`): for each instruction, record stores with a base register and a constant displacement, direct call and jump edges, indirect calls through `[reg+0x210]` (UObject::ProcessEvent, vtable slot 66), and comparisons of `byte [reg+0xF0]` (AActor::Role, offset from reflection) with an immediate.
3. Store matching: each replicated property maps to a displacement. Bools use `bool_bits.json` (offset, mask, form). A store matches if the displacement matches and the width fits the property kind. Struct, array and similar kinds match on the base offset only.
4. Writer attribution (`analyze.py`): a function is attributed to a class through (a) its native exec thunk (native_funcs), (b) its manifest address, (c) the single exec thunk that calls it, or (d) caller propagation over three rounds. Attribution covered about 56k functions.
5. Tiers per property: `writer_found` = an exact-size store in a function attributed to the owner class or a related class (ancestor or descendant). `writer_candidate` = store sites exist but none is attributed; sites are ambiguous because small displacements are written all over the binary. `bulk_init_only` = stores appear only in initialiser-like functions (one function writes eight or more replicated properties of one class). `none_found` = no store at that displacement and width.

### Known defects in this run (must be fixed before the counts are trusted)

- **Thunk edges were dropped (scan_b).** Exec thunks that sit inside a larger `.pdata` range had their call and jump edges lost, so exec-to-body attribution is incomplete. Example: `AFortGameModeAthena::JumpToSafeZonePhase` exec `0x1428f0420` is a short thunk that tail-jumps to `0x141950090`; that edge is missing in scan_b.
- **Fix written, not finished.** `scan.py` now splits blocks at every native exec address (scan_c). The rescan was started and stopped for wind-down; its output at `scratchpad/scan_c/` is not analysed. Rerun `analyze.py` and `picks.py` on `scan_c` to finish.
- **RPC send stubs not located.** The 2,691 functions that call `[reg+0x210]` include the Client/Multicast stubs, but nothing in the repo or the scan maps a stub to its RPC name. The method idea is to read the static FName or UFunction lookup in each stub; this was not done. Client/Multicast writers are therefore reported as `not_located`, not `none_found`.
- Attribution is weak for helpers. Only about 11% of functions get a class.

## 3. Per-area counts (scan_b, PARTIAL)

Area assignment is keyword-based (first match on the observable name, then on the class name). It is a heuristic.

| Area | Props | writer_found | writer_candidate (ambiguous) | bulk_init_only | none_found | Client+Multicast RPCs (not located) | Found-tier writer fns | of which ported |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| bus / storm / match flow | 271 | 11 | 247 | 9 | 4 | 21 | 12 | 0 |
| elimination / damage / health | 167 | 9 | 156 | 0 | 2 | 27 | 10 | 1 |
| building | 136 | 12 | 121 | 0 | 3 | 11 | 13 | 0 |
| inventory / loot / pickups | 179 | 12 | 166 | 1 | 0 | 20 | 14 | 0 |
| abilities / GameplayCues | 87 | 17 | 70 | 0 | 0 | 30 | 18 | 0 |
| AI / bots | 72 | 5 | 54 | 2 | 11 | 11 | 6 | 0 |
| UI / stats / HUD | 252 | 11 | 225 | 7 | 9 | 42 | 12 | 0 |
| session / party / social | 86 | 3 | 77 | 3 | 3 | 15 | 3 | 0 |
| movement / pawn | 180 | 23 | 152 | 2 | 3 | 12 | 23 | 0 |
| other | 285 | 16 | 261 | 0 | 8 | 51 | 22 | 0 |
| **Total** | **1,715** | **119** | **1,529** | **24** | **43** | **240** | | |

The table above is copied from the scan_b analysis (its total is 119/1,529/24/43, from the second analysis run). The "Props" column per area is from `area_counts` in the JSON. Columns for writer counts come from `analysis.json`; the "ported" column counts the found-tier writer functions that match a `ported` manifest address (249 ported entries in `MANIFEST.json`).

Reading the table: almost every property has store sites, but very few are attributed to an owner class. The found tier is small, and it is mostly storm-radius, storm-shield, trap and pickup stores in exec-attributed functions. Among the found-tier writer functions, only one (an elimination-area function) is a ported body; the other 249 ported manifest bodies were not checked against the full candidate lists. The ported bodies are mostly RPC impl/validate and OnRep bodies, which are receiver-side.

## 4. Server behaviors to port first (ranked; exe locations)

Exec addresses come from `native_funcs.json` (UFUNCTION exec thunks). Body addresses are not resolved for most of these, because of the thunk defect above. "Ported" means the class and function pair is in `MANIFEST.json` as ported; I did not verify that for each row, so treat it as unchecked.

Ranking reflects match-flow value: storm and phase, bus and drop, loot and pickups, AI, match end, then elimination and health.

| # | Behavior | Exe location (exec thunk) | Body / callees | Data assets (paks) | Confidence |
|---|---|---|---|---|---|
| 1 | Storm phase advance | AFortGameModeAthena::JumpToSafeZonePhase `0x1428f0420` | body unresolved (tail-jump to 0x141950090) | Content/Athena/SafeZone/BP_SafeZoneStorm, SafeZoneIndicator, SafeZoneCylinder | high (name), medium (body) |
| 2 | Storm inside checks and damage | AFortGameModeAthena::SafeZoneInsideChecks `0x1428f08d0` | decomp at `porting/generated/decomp_improved/AFortGameModeAthena/SafeZoneInsideChecks.c` | GE_OutsideSafeZoneDamage, GE_OutsideSafeZoneDamageCN | high |
| 3 | Phase step change fan-out | AFortGameModeAthena::OnGamePhaseStepChanged `0x1428f0670` | body `0x141953840` (82 B) and two shared stubs | none | medium |
| 4 | Safe-zone radius and center | AFortSafeZoneIndicator::SetSafeZoneRadiusAndCenter `0x142ad2c40` | callees small; storm radius writer `Radius` (Float) attributed to `0x141a5b1f0` | Content/Athena/SafeZone/SafeZoneIndicator | medium |
| 5 | Storm radius setter (mission) | AFortMissionStormSafeZone::SetSafeZoneRadius `0x142765ea0` | body `0x142765eef` | Content/Missions/Primary/StormShield | low |
| 6 | Storm shield spawn and radius | AFortMissionStormShield::SpawnSafeAreaAtLocation `0x1429f0d40`; SetRadiusOfShield `0x1429f07c0` | small callees | Content/Missions/Primary/StormShield | medium |
| 7 | Storm progression pause (mutator) | AFortAthenaMutator_Storm::PauseStormProgression `0x142828250` | callees `0x1428282a0`, `0x141b28390` | none | low |
| 8 | Drop-zone enter and exit | AFortGameModeAthena::OnAircraftEnteredDropZone `0x1428f0440`; OnAircraftExitedDropZone `0x1428f04c0` | body `0x1419520f0` (160 B) and `0x141952190` (skipped in manifest) | Content/Athena/Aircraft/PartyBus_RT, PartyBus_Birthday (battle-bus BP not found by name) | medium |
| 9 | Flight end | AFortGameModeAthena::OnAircraftFlightEnded `0x1428f0540` | body `0x141952500` | Aircraft assets as above | medium |
| 10 | Aircraft creation | AFortMissionAircraft::CreateAircraft `0x1429e4670` | body `0x1420b2110` (180 B) | Aircraft assets | low |
| 11 | Loot removal and re-spawn | AFortGameModeAthena::RemoveAllLootFromWorld `0x1428f0750`; StopRemovingAllLoot `0x1428f0970` | body `0x14195e060` (58 B) | Content/Athena/Playlists/OverrideLootTierData, OverrideLootPackagesData | medium |
| 12 | Supply-drop pickup spawn | AFortAthenaSupplyDrop::SpawnPickup `0x142834e20`; SpawnPickupFromItemEntry `0x142834fb0` | callee `0x141922270` | Content/Athena/SupplyDrops/AthenaSupplyDrop | medium |
| 13 | Supply-drop despawn on storm | AFortAthenaSupplyDrop::OnDespawnDueToStorm `0x142834730` | callee `0x14191c710` (52 B) | Content/Athena/SupplyDrops | low |
| 14 | Supply-drop loot landed | AFortAthenaSupplyDrop::OnLootSpawned `0x1428347d0` | callee `0x14191d000` | Content/Athena/SupplyDrops | low |
| 15 | Bot spawn | UFortServerBotManagerAthena::SpawnBot `0x142ad7380`; SpawnAI `0x142ad7250` | callees `0x141a5ca90` (2,270 B), `0x141a5c280` (1,936 B) | Content/AI/BehaviorTrees/MissionBotBTs (bot playlist not verified) | medium |
| 16 | Bot kill and cleanup | UFortServerBotManagerAthena::KillBots `0x142ad7010` | callee `0x141a4b6c0` | none | low |
| 17 | AI director encounter | AFortAIDirector::StartEncounter `0x142794fb0`; Activate `0x142737e10` | direct only | Content/AIDirector/DataTables/AIDirectorSpawnBudgetCurves, AIDirectorEnemyVariantProgressionCurves, AIDirectorRespawnCurves | medium (name) |
| 18 | Bot reaction to storm | AFortAthenaAIBotController::OnSafeZoneUpdated `0x1427c2f20`; OnSafezoneStateChanged `0x1427c2f40`; UFortAthenaAIBotEvaluator_Storm::OnSafezonePhaseChanged `0x1427c9110` | callee `0x1416a5b00` (67 B) for OnSafeZoneUpdated | none | low |
| 19 | Match end and kick | AFortGameModeAthena::OnEndGameKickPlayers `0x1428f0650` | body unresolved (thunk defect) | none | low |
| 20 | Orphaned-server check | AFortGameModeAthena::OnCheckForOrphanedServer `0x1428f05c0` | body not resolved | none | low |
| 21 | Health and shield replication | AFortGameModeZone::ReplicateHealthAndShield `0x1428f3600` | body unresolved | none | low |
| 22 | Storm-shield spawn gating | AFortGameModeZone::SetSpawnWithinStormShield `0x1428f3b70`; SetActiveSpawnPad `0x1428f3620` | callees `0x141eb0870` | none | low |
| 23 | Kill report to client (RPC) | AFortPlayerStateAthena::ClientReportKill `0x142a8d5c0` (Client RPC; skipped in manifest) | callee `0x142a8d611` | none | low |
| 24 | Bus for player and aircraft lookup | AFortGameStateAthena::GetBattleBusForPlayer `0x14291e820`; GetAircraft `0x14291e730` | getters | Content/Athena/Aircraft | low |
| 25 | Storm progression on game-state | AFortGameModeAthena::SafeZone phase writer (GameState SafeZonePhase, Byte at 0x1eb9) | single store candidate at `0x14197f7b0` (not attributed) | none | low (guess) |

Elimination, the kill and death writes to PlayerState (Kills, Downs, DeathInfo), and the win condition writes to GameState (WinningTeam, WinningPlayerState) have **no attributed writer** in this run. Their candidates are ambiguous or none. OnRep bodies for Kills, Downs and AIKills are ported; their server writers are not located.

## 5. Areas with no server logic found in the exe

Stated as no logic found by this method, not as proof that none exists:

- **Battle bus blueprint and bus movement**: no bus blueprint asset was found by name in the extraction, and no bus movement writer was attributed. Only drop-zone UFUNCTIONs were found.
- **Win condition and match-end writers on GameState** (WinningTeam, WinningPlayerState, GamePhase): candidate stores only.
- **Elimination and death server writers** (PlayerState Kills, Downs, DeathInfo, damage application): not attributed.
- **Client and Multicast RPC send stubs** (240 observables): not located; no caller analysis was possible.
- **Loot spawn table evaluation**: asset names exist (OverrideLootTierData, OverrideLootPackagesData) but no exe writer was attributed.
- **Replicated subobject classes**: `ReplicateSubobjects` is not present in the generated headers (grep found none); this was not analysed further.

## 6. Exact next steps

1. Rerun on the corrected scan: `python3 -I scripts/analyze.py` and `scripts/picks.py` against `scratchpad/scan_c/` (the scan itself: `objdump -d -M intel --no-show-raw-insn FortniteClient-Win64-Shipping.runtime.exe | python3 -I scripts/scan.py`, or `python3 -I scripts/scan.py < full.asm`). Expect body addresses to resolve for the thunks listed above.
2. Map RPC send stubs: find the stub for each Client/Multicast RPC (functions calling `[reg+0x210]` with a static FName or UFunction lookup), then list their direct callers outside RPC bodies.
3. Attribute elimination, loot and win writers by reading the object-to-class relation (type_refs in the reflection file), rather than relying on exec-thunk attribution alone.
4. Cross-check the 24 found-tier writer functions against `MANIFEST.json`.
5. Decompile the selected server bodies (only with the existing Ghidra slim configuration; no new runs in this pass).

## Changed files

- `porting/server_authority_inventory.md` (this file, new)
- `porting/generated/server_authority_inventory.json` (new, git-ignored)
- Scripts and README copied to `/opt/fortnite/local-model-package/inventory-wip/` (outside the repo)

No file under `porting/native/`, `porting/server/`, `porting/cue4inspect/`, the census or the extraction trees was modified. Nothing was committed or staged.
