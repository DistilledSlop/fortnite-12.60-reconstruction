# Getting the vanilla client into the world: map first, then fix

Written 2026-11-12 during G6. This is a perspective on how the work could go faster. It is not a task list.

## What happened

The client stayed on the LOADING screen after it joined our server, and nothing in its log said why. The loading screen logs a reason only when it turns on or off, so once the join finished there was silence. I found the missing server pieces one at a time. Each cycle was: guess from one decompiled function, change the server, build (about 2 minutes), run the client (about 10 minutes at 2 FPS), look. Several guesses were wrong because I misread a single function in isolation. The fixes were small. Finding which one was missing took most of the time.

The pieces found this way, in order:

1. `ClientOnGenericPlayerInitialization` never ran, so the client's readiness timer was never armed.
2. The player had no hero or character parts, so the pawn customization wait never finished.
3. No time-of-day manager, and the game phase was not Warmup.
4. No WorldManager on the GameState. Without it the client's UI state tag stays `None`, and the loading manager reports "UIDirector has no valid state" forever.

With those in place the loading screen dropped and the Battle Royale HUD appeared (run 26).

## The idea

The client's readiness logic is entirely in the client binary, which we have. The decisions that keep the loading screen up are a finite set:

- the loading-screen manager decision (`0x141f062e0`)
- the Athena controller override (`0x1419f9d30`) and its volume helper (`0x1419fa3b0`)
- the UI state tag function (`0x14514b2f0`) and the updater that calls it (`0x1451755b0`)
- the WorldManager state machine (`0x141f9da60`)

A pass that reads all of these up front and writes down every condition, with the object field and the value it needs, would give a checklist. A probe could then print the live value of every condition in one run, and every failing one could be fixed together. That replaces several guess, build and run cycles with one.

The same pass could ask the reverse question. Instead of "why is the screen still up", ask "what must GameState, PlayerState, PlayerController and the world hold for the client to consider itself in a match". The answer is a list of actors and fields the server has to provide, such as the managers (time of day, world, volume), team info, the phase, mutators and the player's hero.

## What this would and would not do

- It would not remove the need to run the client. Whether the screen actually drops can only be seen by running it.
- It would not make the server logic original. The server-only code is compiled out of the shipped client, so what we build is behavior the client accepts, inferred from what it demands. It will differ from the real server wherever the client does not constrain it (storm timing, bots, match rules).
- It is most valuable where the failure is silent. A heap scan for the live reason string and a probe of the objects behind each condition turned silence into a specific answer. Doing that earlier would have saved several runs.

## Things that cost time and are worth knowing

- Reason strings in the override are stored as inline UTF-16 constants, so searching the binary for string literals misses them. Decode them from the store instructions.
- A native bitfield in a class can be named and positioned wrongly by assumption. The bit masks are in `generated/reflection/bool_bits.json`.
- A property flagged `Transient` on a replicated class is computed by each side on its own (for example the WorldManager state), so seeing the server's value says nothing about the client's.
- Run progress can be judged from the client's heap and object state through `tools/pc_probe.py`. The client log alone is not enough.

## Open questions this leaves

- Whether the volume manager, team info and mutators need real server objects for the match to behave beyond the HUD appearing, or only for later gates.
- How much of the warmup and time-of-day behavior should be ported properly versus kept as stand-ins.
- Whether the movement gate (the pawn responds to input and stays in the world) holds without further server pieces. Run 27 tests this.

## Function and control-flow map, G6 to G9

**Added 2026-11-12, after the detailed read.** The real-code parts of this map
now have line-by-line cards: `server_systems/R1-warmup-and-aircraft-code.md`,
`R2-safe-zone-code.md`, `R3-elimination-and-match-end-code.md`,
`R4-readiness-build-interact-code.md`. Implement from those. They correct
four things in the map below:

- `StartEndGamePhase` starts at `0x14196cb30`, not `0x14196cb50`.
- `ServerCreateBuildingActor` is not real code. `0x1423aad00` is its
  implementation and holds only a guard; `0x14236d1a0` is a different helper.
  Building has to be re-created (R4 section 2).
- GamePhase numbers are Warmup 2, Aircraft 3, SafeZones 4, EndGame 5.
- `StartAircraftPhase` destroys each warmup pawn and puts the controller in
  the Spectating state. `PlacePlayersInAircraft` then runs from the
  phase-change handler when the phase becomes Aircraft, and the player gets a
  pawn again through `RestartPlayer` (R1 section 6, R5 sections 3 and 4).

`server_systems/R5-playlist-setup-phase-handler-receipts.md` covers the flight
path and storm-center setup, the phase-change handler, bus placement and the
client receipts.

### G6: in the world with a controllable pawn

Client side (read). Control flow from join to a visible HUD:

1. Server sends the PlayerController. The client RPC `ClientOnGenericPlayerInitialization` (impl `0x142371f00`) runs and, through controller slot `+0x1540` (`0x1423dd5c0` then `0x14239a890`), arms a 0.5 s looping timer whose callback is `0x1423bc750`.
2. The timer calls the readiness check `0x1423cc220`, then `SetHasFinishedLoading` (`0x1423af810`). `OnReadyToStartMatch` follows and creates the ClientQuickBars.
3. The pawn customization poll (`0x1423593a0`) waits until the PlayerState's CharacterData holds valid parts. A hero type must be set or it logs `Hero:None`.
4. The loading-screen manager decision (`0x141f062e0`) runs each tick and keeps the screen up for the first matching reason: no GameState, world not begun play, UI manager not initialized, **UIDirector has no valid state**, matchmaking pending, WorldManager not initialized, streaming levels loading, mutator request, waiting for time of day manager, loading user content. Each Athena controller also votes through slot `+0x14f0` (`0x1419f9d30`): not ready to start match, disconnecting, no GameState, pre-begin-play, no pawn location, in warmup with levels still loading.
5. The UI director's state is a gameplay tag (`UI.State.Athena.*`). The updater `0x1451755b0` calls the Athena manager's tag function `0x14514b2f0`, which returns BattleRoyale only if the GameState is an in-game state, `GameState+0x3d0` (WorldManager) exists, and its state byte `+0x2c8` equals 5. The WorldManager state machine (`0x141f9da60`) advances that byte on the client by itself, but only if the actor exists.

Server side (what we provide, all stand-ins so far): the `ClientOnGenericPlayerInitialization` call, hero and character parts on the PlayerState, a time-of-day manager, GamePhase set to Warmup, and a replicated WorldManager on the GameState. Missing for full fidelity: the real `InitGame` (`0x14194cd90`), the real `StartWarmupPhase` (`0x141972c30`), a volume manager (`GameState+0x2108`), team info.

The controllable-pawn half is checked by holding keys and comparing the pawn's server position over time (run 27).

### G7: match flow (warmup, bus, jump, glide, land, storm)

Server control flow (read, in the client binary):

- `InitGame` (`0x14194cd90`): defaults (80 required players, 60 s countdown) and URL options such as `RequiredPlayers`, `SkipWarmup`, `PauseWarmup`, `SkipAircraft`.
- `HandleMatchHasStarted` (`0x14194bb50`): reads the playlist, calls `StartWarmupPhase`, spawns the MegaStormManager.
- `StartWarmupPhase` (`0x141972c30`) sets GamePhase Warmup. `UpdateWarmupPhase` (`0x14197a400`) runs on a timer: sets the countdown end time, shortens it when enough players have joined, and at zero starts the aircraft phase.
- `StartAircraftPhase` (`0x14196b640`): spawns or takes aircraft, binds three delegates (entered drop zone, exited drop zone, flight ended), places players (`PlacePlayersInAircraft`, `0x1419a8ec0`, which needs the replicated `Aircrafts` array first), then sets GamePhase Aircraft.
- `StartSafeZonesPhase` (`0x141971aa0`): starts the storm timers from the storm curve tables and sets GamePhase SafeZones.

Compiled out (design): the bus jump request `UFortControllerComponent_Aircraft::ServerAttemptAircraftJump`, the forced exit when the bus leaves the drop zone, and `ServerSetInAircraft`. The skydive and glide movement states are client-driven.

### G8: core loop (pickaxe, harvest, build, loot, shoot, damage, elimination)

Read: weapon and ability RPCs ride on the stock GameplayAbilities plugin; `ServerCreateBuildingActor` (`0x1423aad00` then `0x14236d1a0`); `ServerAttemptInteract` (`0x141d15810`); damage cues to clients (`0x142334fc0`); client death handling `ClientOnPawnDied` (`0x1423cd300`); `RemoveFromAlivePlayers` (`0x14195e620`).

Compiled out (design, the largest re-creation): the server death sequence. Its entry point should be found when porting the pawn's death virtual; the order is death report, victim death info, killer score, `ClientOnPawnDied`, inventory drop, `RemoveFromAlivePlayers`, placement and stats. Also empty: inventory setup at spawn, `ServerExecuteInventoryItem`, `ServerAttemptInventoryDrop`, the pickup RPCs, `ServerBeginEditingBuildingActor` and its edit and repair partners, `ServerOnMaterialSelection`. Loot tables and the rolling rule are in game data.

### G9: match end and return to lobby

Read: `CheckWinConditionLastManStanding` (`0x141937590`), `StartEndGamePhase` (`0x14196cb50`, sets the winner and grants victory), `HandleMatchHasEnded` (`0x14194b750`, sets GamePhase EndGame and clears timers), and the client receipts `ClientNotifyWon`, `ClientSendMatchStatsForPlayer`, `ClientSendEndBattleRoyaleMatchForPlayer` (`0x1419c6120` to `0x1419c7d90`). `ServerReturnToMainMenu` is real code.

Design: filling the placement, stats and reward structures the receipts carry, and the server to backend result report (`POST .../result`, D3 section 4), which is what puts XP and stats back into the profile.

### How the gates connect

G6 only needs the client to believe it is in a match. G7 turns the phase from a fixed stand-in into the real state machine, which needs `InitGame` and the aircraft. G8 needs a player with inventory and a working death sequence, which G9 then ends. The stand-ins added for G6 (phase, time of day) are replaced in G7. The pawn, inventory and PlayerState work done for G6 carries through to G8.
