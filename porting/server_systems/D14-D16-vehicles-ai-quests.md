# D14–D16: vehicles, bots and henchmen, quests and XP (ahead-thinking)

Date: 2026-11-11. Based on the body sweep (`rpc_sweep.py`), the name index and
reflection data. None of these is needed for the first playable solo match
(G5–G9). They are listed so nobody starts them blind.

## D14. Vehicles (12.60: motorboat, helicopter; older classes still present)

What exists (real bodies):

| Class | Real / empty | Notes |
|---|---|---|
| `AFortAthenaVehicle` | 73 / 6 | enter (`ServerOnAttemptInteract`, 844 bytes), fire, fall-damage flags, rift. Empty ones are debug or cosmetic |
| `UFortVehicleInterface` | 71 / 0 | seats, driver/passenger queries |
| `AFortHoagieVehicle` (helicopter), `AFortMeatballVehicle` (boat) and others | all real | `ServerUpdateControlState` etc. |
| `AFortPlayerControllerZone::ServerAttemptExitVehicle`, `ServerRequestSeatChange`; `AFortPlayerPawn::ServerOnExitVehicle`, `ServerUpdateVehicleInputState*` | real | |
| `AFortPhysicsPawn::ServerMove(FReplicatedPhysicsPawnState)`, `ServerUpdateStateSync(TArray<uint8>)` | **empty** | the movement receive path |
| `AFortAthenaVehicleSpawner::SpawnVehicle` | **empty** | |

Abilities `GA_AthenaEnterVehicle_C`, `GA_AthenaExitVehicle_C`,
`GA_AthenaInVehicle_C` are in the default ability set (C card §2).

What must be re-created, and the hard part:

1. **Spawner** (easy): at match start each `AFortAthenaVehicleSpawner` in the
   loaded levels spawns its vehicle class at its transform. The class comes
   from the spawner's properties (read one spawner with the asset tools).
2. **Movement** (hard): physics vehicles are driven on the owning client. The
   client simulates, sends `ServerMove(InState)` unreliably every frame, and
   expects `ClientAckGoodMove` back (that function exists on the class, so the
   protocol is: client sends state with a timestamp, server accepts or
   corrects). The server must (a) apply the received state to the vehicle's
   physics body (position, rotation, linear and angular velocity), (b) answer
   `ClientAckGoodMove(timestamp)`, (c) let the normal replicated movement carry
   it to other clients. Start with accept-everything; add sanity limits
   (max speed, max teleport distance) afterwards. To do this the worker needs
   the layout of `FReplicatedPhysicsPawnState` (reflection struct at
   0x142a26d20) and must read the client's `ClientAckGoodMove` body to see
   which field it matches on.
   `ServerUpdateStateSync` carries a compressed byte blob; first pass: ignore.
3. **Server-side physics**: the dedicated server needs collision for the
   vehicle to rest when nobody drives. Depends on C2 (world collision).

Test: enter boat, drive, second client sees it move; exit; boat stays.

Decision: after G9. Until then do not spawn vehicles (spawners stay idle), so
the client cannot enter a half-working one.

## D15. Bots, henchmen, bosses

What exists: `AFortAthenaAIBotController` (22 reflected, all real, plus 22
string-named C++ functions), `UFortServerBotManagerAthena` (`SpawnBot`
0x141a385a0 / 0x141a5ca90, `SpawnAI` 0x141a5c280, `SetBotTeamFromPlayerPawn`
0x141a4b5e0, `PerformDeadBotsCleanup`), bot evaluators and skill sets, the
spawner components (`UFortAthenaAISpawnerDataComponent_*`), `AFortAIDirector`,
`UFortMangDataTrackerComponent` (henchmen tracker). `AAIHotSpot` has 9 empty
Blueprint-facing events (harmless). In the body sweep: 160 real, 15 empty.

So most AI code is recoverable. The cost is elsewhere:

1. **Navigation.** Bots need a navmesh on the server. The game mode default
   names `AthenaNavSystemConfig`; the cooked map carries nav data in its
   sublevels, or it is built at load. This depends fully on C2 and is the
   largest unknown: check whether `Apollo_Terrain` sublevels contain
   `RecastNavMesh` data (asset tools `--actor-report`).
2. **Behaviour trees and Blueprints.** Bot logic is in behaviour-tree assets
   and `GameplayMod_Phoebe_DM_Solo` / `GameplayMod_DeimosAI` modifiers. They
   must load on the server (same work as C1, another closure).
3. **Henchmen levels.** The playlist's `AdditionalLevelsServerOnly` (`*_MANG`
   levels) hold the spawners for Agency, Rig, Yacht, Grotto, Shark. They exist
   in the content and only load on the server.
4. `ServerBotManagerClass` is None in the game mode default: the manager class
   comes from the AI settings / playlist. Read `Athena_AI_Settings` and
   `Playlist_Bots_DefaultSolo` before starting.

Decision: after G10. A solo playlist without bots is valid for the client:
`PlaylistEnableBots` is a curve value (`Default.Phoebe.Enable`), keep it 0 by
not applying the bot modifiers (D4 card §1).

## D16. Quests, accolades, in-match XP

What exists: `UFortQuestManager` (41 reflected, all real; C++ functions
`InitQuests`, `PopulateQuestLookupMap`, `UpdateStatsForSingleMatchQuests`),
`UFortPlayerControllerAthenaXPComponent` (14, all real: `OnXPEvent`,
`HighPrioXPEvent`, `ClientMedalsRecived`, `ClientQuestsUpdated`,
`OnRep_InMatchProfileVer`, `OnMatchResultsReported`), `ServerAddSimulatedXp`
(real). 273 real and 1 empty across 71 quest/XP classes.

How it fits together (reasoning from the names and replicated fields; verify
by reading `OnXPEvent` and `OnRep_InMatchProfileVer` when this starts):

- The XP component sits on the player controller (C card §1). During a match
  the server raises XP events (kill, chest opened, placement, survival time);
  the component replicates totals and tells the client to show the XP bar and
  medals. Amounts come from `AthenaAccoladeXP` (D4 card §4).
- Quest progress lives in the player's profile. The original server sent stat
  events to the quest manager, which wrote objective counts to the profile
  service; the client then saw a new profile revision (`InMatchProfileVer`)
  and re-read the profile.
- For us: the game server reports events to our backend (extend the `result`
  call in the D3 card to a mid-match `progress` call), the backend updates
  quest objective attributes in the `athena` profile and bumps the revision,
  and the server sets `InMatchProfileVer` so the client re-queries.

Hard parts:

1. The quest definitions (which stat, which tags, how many) are data assets
   (`FortQuestItemDefinition`, challenge bundles). Matching server events to
   objectives means reproducing the tag matching in
   `UpdateStatsForSingleMatchQuests` / `CheckObjectiveStatUsage`: port those,
   do not invent the matching.
2. Where the matching runs. Simplest: run the ported quest manager on the game
   server with the player's profile fetched from the backend at login, and
   send only the resulting objective deltas to the backend.
3. Battle Pass XP at match end already has a backend path
   (`season12_rewards.py`); in-match XP display is cosmetic until that works.

Order: match-end XP (E card, E2) first; in-match XP events second; quest
objectives last.
