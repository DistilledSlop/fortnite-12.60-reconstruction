# How much original server code is in the client (work in progress, 2026-11-11)

Label `binary` unless stated. Image: `generated/dump/FortniteClient-Win64-Shipping.runtime.exe`.
Name index: `server_scope/string_names.json` (built by `tools/nameidx.py`).

## 1. Main result

**Much more of Epic's server logic is present in the client than the earlier notes assumed.** The functions carry their own names in log strings, so they can be identified without guessing. This moves a large part of match flow from "re-create" to "recover".

Identified by log strings (address = `.pdata` chunk that loads the string; the function start is at or shortly before it):

### AFortGameModeAthena (41 functions named)

| Area | Functions and addresses |
|---|---|
| Playlist | `ApplyPlaylistDataTables` 0x141934980 / 0x1419350c0, `OnPlaylistDataLoaded` 0x141955618, `SetCurrentPlaylistId` 0x141963c70 |
| Teams | `PickTeam` 0x14195863e, `AttemptChangeTeam` 0x141935140 |
| Spawn | `ChoosePlayerStart_Implementation` (+ `_Aircraft`) 0x14193a360, `ChoosePlayerStart_Creative` 0x141938740, `FinishRestartPlayer` 0x141943240 |
| Phases | `StartWarmupPhase` 0x141972c30, `UpdateWarmupPhase` 0x14197a400, `StartAircraftPhase` 0x14196b7ca, `OnAircraftEnteredDropZone` / `ExitedDropZone` / `FlightEnded` 0x14195252a, `StartSafeZonesPhase` 0x141971aa0, `IsSafeZoneAllowed` 0x141971930, `IsInsideSafeZone` 0x14194f529, `InitializeStormCapSystem` 0x1419559de, `OnGamePhaseStepChanged` 0x14194d872, `StartEndGamePhase` 0x14196cb95, `StartEndGamePhaseTeam` 0x14196d1f6 |
| Match start | `HandleMatchHasStarted` 0x14194bb86 (logs "NumPlayers", "Skipping Warmup due to playlist settings", "Skipping Aircraft due to playlist settings"), `OnGivenMatchAssignment` 0x14194c60a |
| Win and loss | `CheckWinConditionLastManStanding` 0x141937590, `GrantVictoryForNumTeamsAndEndGame` 0x14194acc5, `GrantVictoryToAllRemainingTeams` 0x14194b0a0, `GrantLosses` 0x14194a7d6, `GrantLossToAllRemainingTeams` 0x14194a6d0, `GrantLossToAllTeams` 0x14194a720, `CheckEndMatchOnLogout` 0x14195e9e7 |
| Loot tuning | `GetLootChanceMultiplier` 0x141947876, `GetLootCountMultiplier` 0x141947a96 |
| Server lifecycle | `OnDedicatedServerRestarting` 0x141952cf0, `StartMatchAbandonProcess` 0x14196d920, `OnMatchAbandonTimeout` 0x141954350, `CreateAIDirector` 0x14193b020 |

### Other server-side classes

| Class | Functions named |
|---|---|
| `AFortGameStateAthena` (22) | `PlacePlayersInAircraft` 0x1419a8f1b, `LoadCurrentPlaylistData` 0x14199f7df, `OnPlaylistDataLoadCompleted` 0x1419a3bb0, `UpdatePlaylistDependentData` 0x1419b4981, `InitializeMapInfo` 0x14199dcc0, `StartPlaylistTimer` / `StopPlaylistTimer`, `GetAircraft`, `GetAircraftIndexFromTeam`, `AllowItemPickUp` 0x141989960, `OverrideLootLevel` 0x1419a8c50, `CheckAndAddMissedPlayerStatesToMaps` |
| `AFortGameSessionDedicated` (5) | `CreateDedicatedSession` 0x14214fe1b, `Restart` 0x1421742e0, `QueueEndSessionRestart`, `OnAllPlayersUnregistered`, `OnConnectionStatusChanged` |
| `AFortGameSessionDedicatedAthena` (6) | `ProcessPlaylistCommandLine` 0x142159f73, `ParsePlaylistCommandLine` 0x142159d9a, `GetServerMatchmakingBucketIdsV2` 0x14215a45a, `OnMatchmakingSessionBackfillMatchRequestV2` 0x14216977f, `RemovePlayerFromSquadAndTeamMap` |
| `AFortGameModeZone` (16) | `HandleMatchHasStarted` 0x141ea0a48, `HandleMatchHasEnded` 0x141ea0420, `UpdateSessionForMatchmaking` 0x141eb6bec, `OnZoneEnd`, `RemoveKnownPlayer`, analytics |
| `UFortServerBotManagerAthena` (5) | `SpawnBot` 0x141a5ca90 / 0x141a385a0, `SpawnAI` 0x141a5c280, `SetBotTeamFromPlayerPawn`, `PerformDeadBotsCleanup` |
| `ABuildingContainer` | `SpawnLoot` 0x141bec150 (chest loot), `ServerOnAttemptInteract` 0x141be8654, `GetSearchTime` |
| `AFortPlayerController` | `ServerCreateBuildingActor_Implementation` 0x14236d1a0, `AddItemToQuickBars` 0x142368650, `AddInventoryItemWithAnimation`, `OnReadyToStartMatch`, `CanRestartPlayer`, `ServerBroadcastPlayerChangedBuildMode` |
| `AFortInventory` | `SendInventoryChangedNotification` 0x142027b30 |
| `UFortGameData` | `GetBuildingCost`, `GetBuildingInitialHealthPct`, `GetAthenaBuildingInitialHealthPct`, `GetBuildingRepairCostMultiplier`, ... |
| `ABuildingSMActor` | `DetermineMaxResourcesToSpawn` 0x140298ad0 (harvest yield) |

The dedicated-session functions matter for the backend: they show how Epic's server created its session, read the playlist from the command line and built matchmaking bucket ids. That is the reference for our game-server-facing API (plan WP E2).

## 2. Correction: two "stripped" claims in the draft notes are wrong or unproven

- **`ServerCreateBuildingActor_Implementation` is not a no-op.** A 143-line body at `0x14236d1a0` logs under its own name ("BuildPreviewMarkerExtraPiece didn't build when it should have"), reads runtime options, builds spawn parameters (`0x144680860`) and calls `0x141bc5dc0` and `0x1423881c0`. The draft (chunk 17) looked at a different target (a base-class slot or the wrong override).
- **`ABuildingContainer::SpawnLoot` exists** at `0x141bec150` (135 lines) and also calls `0x141bc5dc0` with spawn parameters.

`0x141bc5dc0` is shared by both and has now been read (50 lines): it takes world, class, transform and spawn parameters, optionally overrides the target level through the game state (Creative island lookup `0x141c74820` / `0x141c7c1f0`), and calls `0x1442e6580`, a full engine function (`UWorld::SpawnActor`), not a stub. **So building placement and chest loot spawning are real, recoverable server code.** Treat every "STRIPPED" label in `server_scope/local_model/README.md` as unverified. The four shared stubs are real, but a draft that reached a stub may have followed the wrong virtual target.

Rule for workers: before calling a function stripped, (1) look it up in `server_scope/string_names.json`, (2) resolve the Athena override of the virtual slot, not the base class slot, (3) capture the `_Implementation` body, not only the exec thunk.

## 3. Correction: names in `engine_trace/needed_engine_driven.json` are shifted

That file names each body by the slot name of our 4.25 Linux build. The 12.60 Windows vtable order differs, so many names are wrong. The **addresses are valid** (they are real overrides in the five Athena classes); the names must come from content. Real identities read from strings in each body (`server_systems/d1_bodies.json` has line counts and callee counts):

| Address | Name in the trace file (wrong) | Real identity (from strings) | Lines |
|---|---|---|---:|
| 0x14194cd90 | GetDefaultPawnClassForController | **`InitGame`**: parses `numplayers`, `RequiredPlayers`, `SkipWarmup` options | 444 |
| 0x14194d820 | InitGame | game-state setup that binds `OnGamePhaseStepChanged` (likely `InitGameState` / `PostInitializeComponents`) | 72 |
| 0x14194bb50 | HandleMatchIsWaitingToStart | **`HandleMatchHasStarted`** ("NumPlayers", skip warmup, skip aircraft) | 473 |
| 0x14194b750 | HandleMatchHasStarted | **`HandleMatchHasEnded`** (sets phase to EndGame) | 157 |
| 0x14195a020 | GameWelcomePlayer | login option parsing: `SpectatorOnly`, `BroadcastSpectator`, `FriendSpectator`, `PartyMemberSpectator` (likely `Login` / `InitNewPlayer`) | 168 |
| 0x141959680 | Login | **`PostLogin`** ("Player [%s] PostLogin", `PlayerJoin,%s,%d,%d`) | 201 |
| 0x141950f10 | PostLogin | **`Logout`** (`PlayerLeft,%s,%d`) | 308 |
| 0x141950bd0 | PreLogin | handles `SplitscreenCount`, `NoMCPAccessToken`, `AuthTicket=%s`, `Name` (login URL handling; exact function to confirm) | 131 |
| 0x14193a360 | FindPlayerStart | **`ChoosePlayerStart_Implementation`** | 128 |
| 0x141943240 | ShouldSpawnAtStartSpot | **`FinishRestartPlayer`** (teleport to safe volume logic) | 347 |
| 0x141967ce0 | SpawnDefaultPawnAtTransform | **`SpawnDefaultPawnFor`** ("Failed due to no player controller / start spot / pawn class") | 73 |
| 0x14194d9e0 | UpdateGameplayMuteList | spectate-friend travel option handling | 71 |
| 0x141975090 | CheckStillInWorld (GameMode) | writes `MatchTime`, `GamePhase` (analytics or replication tick) | 132 |
| 0x14198e650 | BeginPlay (GameState) | **`EndPlay`** ("GS EndPlay", stops playlist timer) | 187 |
| 0x1419aceb0 | AddPlayerState | **`RemovePlayerState`** | 278 |
| 0x1419eabb0 | OnPossess | pawn-lost handling ("Removing %s from alive players, pawn was destroyed without dying") | 73 |
| 0x1419e4f50 | Possess | binds `OnFortPlayerPawnAthenaDied` (possess/acknowledge) | 63 |
| 0x1419cef10 | BeginPlay (controller) | binds `HandlePlayersLeftChanged`, volume and server-node delegates | 257 |
| 0x141a4d420 | OnDeactivated | **`SetDisconnected`** | 49 |
| 0x141a1fc00 | PreInitializeComponents (pawn) | **`PostInitializeComponents`** | 161 |
| 0x141a0fcb0 | RegisterActorTickFunctions (pawn) | **`Destroyed`** ("%s destroyed Role: %d") | 43 |

The other bodies have no identifying string; name them by reading, or by re-deriving the Windows vtable order from engine classes with known strings.

Total size of all 59: about 6,600 decompiled lines. Only three call a shared stub (one each, incidental). None jumps into the protected section. **The login, spawn, match-start and match-end path is recoverable code.**

## 4. What this changes

| Topic | Before | Now |
|---|---|---|
| Match phases (warmup, aircraft, safe zones, end game) | "some drafts", unknown bodies | Named functions located; recover |
| Win condition and victory/loss grants | "server writers not located" | Located (`CheckWinConditionLastManStanding`, `GrantVictory*`, `GrantLoss*`) |
| Team assignment | unknown | `PickTeam`, `AttemptChangeTeam` located |
| Putting players in the bus | unknown | `PlacePlayersInAircraft` located |
| Building placement | "stripped, re-create" | Real `_Implementation` body exists; verify the spawn helper |
| Chest loot | "stripped, re-create" | `ABuildingContainer::SpawnLoot` exists; verify the spawn helper |
| Server session and playlist selection | not considered | `AFortGameSessionDedicated*` located |
| Bots | located | unchanged |
| Supply-drop pickup spawn, item toss, XP | stripped | unverified; re-check with the rule in section 2 |

## 5. Next reads (in this order)

1. `0x141bc5dc0`, `0x1423881c0` (shared spawn helper and building placement core).
2. `InitGame` 0x14194cd90, `HandleMatchHasStarted` 0x14194bb50, `StartWarmupPhase`, `UpdateWarmupPhase`, `StartAircraftPhase`, `PlacePlayersInAircraft`, `StartSafeZonesPhase`: write the match-flow state machine from them.
3. `PostLogin` 0x141959680, login option parsing 0x14195a020, `PickTeam`, `ChoosePlayerStart`, `SpawnDefaultPawnFor`, `FinishRestartPlayer`: write the join-to-pawn sequence.
4. `CreateDedicatedSession`, `ProcessPlaylistCommandLine`, `GetServerMatchmakingBucketIdsV2`: derive the server command line and the session fields for the backend.
5. Rebuild the vtable name mapping for the five Athena classes on the Windows layout.
