# D1/D2: join sequence and match-flow state machine (read from the client binary)

Date: 2026-11-11. Source: `python3 -I porting/tools/rd.py ADDR AFortGameModeAthena --brief`
on the runtime dump. All addresses are in the client image (base 0x140000000).
Offsets are **Windows client offsets** of `AFortGameModeAthena` unless another
class is named. Our Linux server project must use reflected property names, not
these offsets.

Main conclusion: **the whole match-flow state machine is present in the client
as real code** (bucket R). Nothing here needs to be re-imagined. A worker ports
each function below from its decompile; the order and the conditions are given
here so the worker can check the port against the intended behaviour.

How sure: structure and strings are read directly. Field names marked `?` are my
inference from use, not from reflection.

## 1. Fields used by the flow

| Offset | Name | Notes |
|---|---|---|
| +0x270 | GameSession | |
| +0x278 | GameState | cast to AFortGameStateAthena everywhere |
| +0x3d8 | FortGameSession | vfn +0x7e8 called at aircraft start (session update, e.g. stop accepting joins) |
| +0xac1 | bAlwaysDBNO | URL `ForceDBNO` |
| +0xb1c bit2 | bAllowDamageInWarmup? (bitfield) | read in FinishRestartPlayer |
| +0xb84 | OverloadedInitialConnectTimeout | copied to GameSession in InitGame |
| +0xb90 | timer handle | cleared in HandleMatchHasStarted |
| +0xb98 | OrphanedServerCheckTime | timer set when match starts with 0 players |
| +0xba0 | timer handle | cleared on first login (function 0x141950bd0) — the "nobody joined" timer |
| +0xcf8 | timer handle | cleared in HandleMatchHasEnded |
| +0xd10 bit0 | bSkipWarmup | URL `SkipWarmup`, or playlist |
| +0xd10 bit1 | bPauseWarmup | URL `PauseWarmup`; also forced from UFortRuntimeOptions byte +0xa4 |
| +0xd10 bit2 | bSkipAircraft | URL `SkipAircraft`, or playlist |
| +0xd10 bit3 | bPauseAircraft | URL `PauseAircraft` |
| +0xd10 bit4 | bSkipSafeZones | checked in StartSafeZonesPhase |
| +0xd11 bit0 | (spectator related) | read in PostLogin |
| +0xd11 bit1 | bDisableEndGamePhase | URL `DisableEndGamePhase` |
| +0xd11 bit2 | bEarlyCountdownStarted? | set when early countdown is triggered |
| +0xd1c | MaxTeamSize? | URL `MaxTeamSize` (PickTeam uses it as team capacity) |
| +0xd20 | TeamSpread? | URL `TeamSpread` |
| +0xd24 | SquadSizes | URL `SquadSizes` |
| +0xd38 / +0xd3c | ForceKickAfterDeathMode / Time | URL options |
| +0xd78 | player count at aircraft start? | written when warmup ends |
| +0xd80 / +0xd88 | Aircrafts array / count | |
| +0xdc0 | timer handle | set in HandleMatchHasEnded (post-match shutdown timer) |
| +0xdc8 | MaxPlayerCount | cmd line `MaxPlayers=`, URL `MaxPlayers` |
| +0xdd0 / +0xdd8 | AlivePlayers array / count | |
| +0xdf0 | TeamAlivePlayers[] (int per team) | |
| +0xf80 | bAircraftPhaseStarted? (byte) | set once in StartAircraftPhase; checked in login ("Match is full"/late join path) |
| +0xf84 | WarmupRequiredPlayerCount | default 80 |
| +0xf88 | WarmupCountdownDuration | default 60.0 (15.0 in the alternate branch) |
| +0xf8c | WarmupEarlyRequiredPlayerPercent | from DefaultWarmupEarlyRequiredPlayerPercent, or playlist |
| +0xf90 | WarmupEarlyCountdownDuration | |
| +0xf94 | bSafeZoneActive (bitfield) | |
| +0xfa0 | GE_OutsideSafeZone (class) | applied in FinishRestartPlayer when zone is active |
| +0xfe0 | SafeZonePhase | |
| +0xfe8 | SafeZoneIndicator | actor; +0x250 start time, +0x254 end time, +0x258 bPaused |
| +0x1048 / +0x1050 | LiveSpectators array / count | |
| +0x1090 / +0x1098 | aircraft start time (double) + set flag | |
| +0x10a0 | world time when warmup started (float, World+0x548) | |
| +0x14e0 | timer handle | set at aircraft start with 60.0 s (0x42700000); cleared at match end |

GameState (AFortGameStateAthena): +0x12d8 `WarmupCountdownEndTime` (float,
written as `GameState[0x25b]`), +0x14a0 `AirCraftBehavior`, +0x1cb0 `GamePhase`
(1 = Warmup, 2 = Aircraft, 3 = SafeZones, 4 = EndGame; 0 = None/Setup),
+0x1ee0 / +0x1ee8 `Aircrafts` array / count.

## 2. InitGame, 0x14194cd90

1. Sets defaults: WarmupRequiredPlayerCount = 80, early percent from the class
   default, WarmupCountdownDuration = 60.0.
2. Alternate branch (a world/PIE style check through world vfn +0x150,
   0x1446939f0, 0x141f3ad70): countdown 15.0, required players 1, and reads
   `numplayers` from the command line.
3. Command line: `RequiredPlayers=`, `MaxPlayers=`.
4. URL options (the `?Key=Value` part of the map URL): `RequiredPlayers`,
   `SkipWarmup`, `PauseWarmup`, `SkipAircraft`, `PauseAircraft`,
   `DisableEndGamePhase`, `ForceDBNO`, `MaxPlayers`, `MaxTeamSize`, `TeamSpread`,
   `SquadSizes`, `ForceKickAfterDeathTime`, `ForceKickAfterDeathMode`.
5. Copies OverloadedInitialConnectTimeout into the GameSession; clears
   bFlightPathInitialized.

**Use for us now:** for single-client tests start the server with
`?RequiredPlayers=1` (or `-RequiredPlayers=1`). `?SkipAircraft` and
`?PauseWarmup` give cheap test modes for G5/G6: `PauseWarmup` keeps the client
on the warmup island forever, which is exactly the G6 "5 minutes stable" test.

## 3. HandleMatchHasStarted, 0x14194bb50

1. Calls the parent, logs NumPlayers (vfn +0x6a0 = GetNumPlayers).
2. Clears timer +0xb90. If no players: sets the orphaned-server timer
   (OrphanedServerCheckTime).
3. Reads the playlist: may set bSkipWarmup / bSkipAircraft ("Skipping
   Warmup/Aircraft due to playlist settings"), time-of-day override
   (TimeOfDayOverrideList, bUseRandomTimeOfDay), backfill blacklist.
4. Calls **StartWarmupPhase**.
5. Spawns MegaStormManager from MegaStormManagerClass; processes
   HotfixKillVolumes / BlockingVolumes / UndergroundVolumes.
6. Sets the live-replay discovery timer.

## 4. StartWarmupPhase, 0x141972c30

1. Needs GameState (AFortGameStateAthena). Calls IsSafeZoneAllowed
   (0x141971930).
2. If bSkipWarmup: log "Skipping warmup phase", go straight to aircraft.
3. Else: store world time in +0x10a0, set GamePhase = Warmup
   ("Set AthenaGamePhase to [Warmup]."), copy the PauseWarmup runtime option,
   copy the playlist early-percent into +0xf8c.

## 5. UpdateWarmupPhase, 0x14197a400 (called on a timer/tick during warmup)

Let `now` = GameState time (vfn +0x630), `end` = GameState.WarmupCountdownEndTime.

- If bPauseWarmup is set: do nothing.
- First time (no countdown yet): `end = now + WarmupCountdownDuration`; log
  "Warmup phase initiated: StartTime, EndTime, NumPlayers".
- If `now >= end`: log "Warmup countdown is over. Attempting to start aircraft
  phase." If GetNumPlayers() == 0: log "All players disconnected during warmup
  phase. Restarting session." and restart. Else store the player count in
  +0xd78 and start the aircraft phase.
- Else if `end - now > WarmupEarlyCountdownDuration` and player count >=
  round(EarlyPercent * MaxPlayerCount): `end = now +
  WarmupEarlyCountdownDuration`, set +0xd11 bit2, log "Player percent reached to
  start early countdown."

Client side: the lobby countdown on the HUD is driven by the replicated
`WarmupCountdownEndTime` and `GamePhase`. Nothing else is needed.

## 6. StartAircraftPhase, 0x14196b640 (param_2 = bool, probably "force")

1. Records the start time (+0x1090, flag +0x1098), sets +0xf80 = 1 once.
2. If no GameState or bSkipAircraft: log "Skipping aircraft phase", jump to
   the safe-zone phase. If already past: log "Already past aircraft phase".
   Playlist null: error.
3. For each aircraft slot (count from the playlist / team setup): take
   `Aircrafts[i]` or spawn one; "Unable to spawn aircraft." on failure.
   Unless bPauseAircraft, bind three delegates on the aircraft:
   **OnAircraftEnteredDropZone, OnAircraftExitedDropZone,
   OnAircraftFlightEnded** (the function at 0x141952500 does the same binding).
4. Builds an analytics-like struct (MaxPlayerCount, players at start, early
   countdown flag, time spent in warmup = world time − +0x10a0).
5. For every player controller (AFortPlayerControllerAthena): calls controller
   vfn +0x1010 (1) and pawn/controller vfn +0x5d0 — the per-player "enter
   aircraft" step. `AFortGameStateAthena::PlacePlayersInAircraft` then puts
   them on the bus.
6. Calls FortGameSession vfn +0x7e8 (session update, joins closed).
7. Calls IsSafeZoneAllowed, sets GamePhase = Aircraft ("Set AthenaGamePhase to
   [Aircraft]."), sets a 60 s timer at +0x14e0.
8. If the match must be abandoned: `StartMatchAbandonProcess` 0x14196d920.

## 7. PlacePlayersInAircraft, AFortGameStateAthena 0x1419a8ec0

Runs on both server and client (logs the local role). Early-outs if the
Aircrafts array is empty or has null entries ("Possibly still replicating").
For each player controller: picks the aircraft index
(`GetAircraftIndex` → `GetAircraftIndexFromTeam` when AirCraftBehavior == 1 and
phase is Warmup/Aircraft; else index 0), then `GetAircraft` 0x141990d90 and
attaches the player.

**Consequence for the server:** the `Aircrafts` array on the GameState must
replicate with valid actors before the phase flips to Aircraft, or the client
logs the early-out and the player stays on the island.

## 8. StartSafeZonesPhase, 0x141971aa0

- If bSkipSafeZones (+0xd10 bit4): log and return.
- Else with SafeZoneIndicator present: un-pause (+0x258 = 0), set
  indicator +0x250 = now + wait time for the current SafeZonePhase,
  +0x254 = +0x250 + shrink time, set +0x258 = 1; set GamePhase = SafeZones.
- Wait and shrink times come from the storm curve tables (see D10 card).

## 9. Elimination → win check → end game

- `CheckWinConditionLastManStanding` 0x141937590(GameMode, RemovedPlayerState,
  Killer?, …): uses `AlivePlayers` and `TeamAlivePlayers[team]`. If the removed
  player's team still has members alive, no win. If only one team has alive
  members: log "Player [%s] is the last player alive. Starting end game." and
  call StartEndGamePhase. Special cases: `Playlist_NowPlaying` is ignored;
  the bots-win mutator logs its own line.
- `StartEndGamePhase` 0x14196cb50(GameMode, WinningPlayerState, Finisher,
  FinishingWeapon…): logs "%s won the match!", resolves EDeathCause, calls
  GameState vfn +0x878 (set winning team/player) and +0x5e8, then the victory
  grant path (`GrantVictory*`, see findings file).
- `HandleMatchHasEnded` 0x14194b750: calls AFortGameModeZone parent
  (0x141ea0420), sets GamePhase = EndGame, sets the +0xdc0 timer, clears
  timers +0xcf8 and +0x14e0, notifies the GameSession.

## 10. Join sequence (G4 → G5)

Order as the engine calls it; all bodies are in the client.

| Step | Function | What it does |
|---|---|---|
| 1 | login option parsing 0x14195a020 | `SpectatorOnly`, `BroadcastSpectator`, `FriendSpectator`, `PartyMemberSpectator`. Rejects with **"Match is full"** when GetNumPlayers() >= MaxPlayerCount. Reads +0xf80 (aircraft already started). |
| 2 | 0x141950bd0 (Login/InitNewPlayer level) | reads URL options `SplitscreenCount`, `NoMCPAccessToken`, `AuthTicket=`, `Name`, `GameAccountId=`; clears the +0xba0 timer on first login. |
| 3 | PostLogin 0x141959680 | log "Player [%s] PostLogin."; spectators go to LiveSpectators; calls a GameSession-level vfn +0x840 (register player); writes the `PlayerJoin,%s,%d,%d` event line. |
| 4 | PickTeam 0x1419585e0 | forced team for mesh-replicated players; otherwise uses the reservation beacon team if present ("used beacon value"), else fills teams up to MaxTeamSize, else "resorting to smallest team". |
| 5 | ChoosePlayerStart_Implementation 0x14193a360 | Creative → ChoosePlayerStart_Creative 0x141938740; Aircraft phase → aircraft start (spectator-team warning); otherwise ChoosePlayerStart_Warmup (warmup island starts). |
| 6 | SpawnDefaultPawnFor 0x141967ce0 | fails with a log line if controller, start spot, default pawn class or world is missing. |
| 7 | FinishRestartPlayer 0x141943240 | if zone active: applies GE_OutsideSafeZone and sets the pawn's zone phase; teleports to the start; on "blocked location" retries, then spawns at safe-volume height. |
| – | Logout 0x141950f10 | `PlayerLeft,%s,%d` event, removes from LiveSpectators, GameSession vfn +0x848 (unregister), cleans the per-player maps at +0x13c0…+0x1458. |

Client URL options that our matchmaking flow must make the client send, or the
server must tolerate missing: `AuthTicket`, `GameAccountId`, `Name`,
`NoMCPAccessToken`. Design choice (mine): the server accepts any `AuthTicket`
in local mode and takes the account id from `GameAccountId`; the backend's
matchmaking ticket should set the same account id it gave the client.

## 11. What a worker does with this

1. Port in this order, one function per commit, each from its full decompile
   (`rd.py ADDR AFortGameModeAthena --full`): InitGame → login options → Login
   level → PostLogin → PickTeam → ChoosePlayerStart → SpawnDefaultPawnFor →
   FinishRestartPlayer → HandleMatchHasStarted → StartWarmupPhase →
   UpdateWarmupPhase. That set is enough for G5 and G6.
2. Then StartAircraftPhase, PlacePlayersInAircraft, OnAircraft* delegates,
   StartSafeZonesPhase (G7).
3. Then the removal/win path and HandleMatchHasEnded (G9).
4. Leave out on first pass (log a TODO, do not invent): analytics structs,
   live-replay timer, hotfix volumes, MegaStormManager, backfill blacklist,
   orphaned-server timer. None of them is seen by the client.
5. Test hooks: `?RequiredPlayers=1?PauseWarmup` for G6;
   `?RequiredPlayers=1` with a short `WarmupCountdownDuration` for G7;
   `?SkipWarmup?SkipAircraft` to reach the storm quickly.

Open reads (not done here): GrantVictory*/GrantLoss* bodies, the three
OnAircraft* handlers, IsSafeZoneAllowed, the safe-zone advance tick, and the
vtable slot names for +0x6a0, +0x840, +0x848, +0x7e8, +0x878, +0x5e8.
