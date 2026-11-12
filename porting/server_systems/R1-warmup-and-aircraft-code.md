# R1: warmup and aircraft phase, read line by line from the client

Date: 2026-11-12. This card is the detailed read of the real server code that
drives G7 up to the moment players leave the bus. It refines
`D2-match-flow.md` sections 4 to 7 and corrects two things in it. Port from
this card and the decompile together; do not guess from one function.

How to reproduce a read: `python3 -I porting/tools/rd.py 0xADDR AFortGameModeAthena --full`.
Addresses are in the shipping exe (image base `0x140000000`). Offsets are
Windows client offsets; our server must use the reflected property names
given here.

Confidence labels used below:

- **read**: the statement is taken directly from the decompile.
- **inferred**: the decompile lost an argument or a name and I filled it in
  from a log string or from how the value is used.
- **not read**: the callee exists but I did not read its body. Its address is
  given so the next reader can.

## 0. Corrections to earlier cards

1. **GamePhase numbering.** `D2-match-flow.md` gives 1 = Warmup, 2 = Aircraft,
   3 = SafeZones, 4 = EndGame. That is wrong by one. The reflected enum
   `EAthenaGamePhase` is None 0, Setup 1, **Warmup 2, Aircraft 3, SafeZones 4,
   EndGame 5**, and the code agrees (`StartWarmupPhase` tests
   `GamePhase != 2` before setting Warmup). Any stand-in that writes the phase
   as a number must use these values.
2. **Who handles a skipped warmup.** `StartWarmupPhase` does not start the
   aircraft itself when `bSkipWarmup` is set. It logs and returns 0; its caller
   (the `HandleMatchHasStarted` chunk at `0x14194bcd0`) then calls the
   warmup-end helper `0x141975ef0` (section 5).

## 1. Names for fields the decompile shows as raw offsets

`AFortGameModeAthena` (bitfield bytes `+0xd10` and `+0xd11` have no reflected
names; the names below are from D2 and from use):

| Offset | Name | Source |
|---|---|---|
| `+0xd10` bit 0, 1, 2, 3, 4 | bSkipWarmup, bPauseWarmup, bSkipAircraft, bPauseAircraft, bSkipSafeZones | inferred from URL option parsing |
| `+0xd11` bit 2 | early countdown started | inferred |
| `+0xd78` | player count stored when warmup ends | read (written from the counter in section 4) |
| `+0xd80` / `+0xd88` | `Aircrafts` array / count | reflected |
| `+0xd90` | number of aircraft that have left the drop zone | read |
| `+0xf80` | set once at first aircraft start | read |
| `+0xf84`, `+0xf88`, `+0xf90` | `WarmupRequiredPlayerCount`, `WarmupCountdownDuration`, `WarmupEarlyCountdownDuration` | reflected |
| `+0xf8c` | warmup early required player percent | read (copied from playlist `WarmupEarlyRequiredPlayerPercent`) |
| `+0xf94` | `bSafeZoneActive` (mask 1) | reflected |
| `+0x10a0` | world time when warmup started | read |
| `+0x10cc` | match time, summed from tick delta | read |
| `+0x1090` / `+0x1098` | aircraft start wall-clock time / set flag | read |
| `+0x14e0` | timer handle, 60 s timer set at aircraft start | read |

`AFortGameStateAthena`:

| Offset | Name |
|---|---|
| `+0x300` | `TeamCount` |
| `+0x12d4` | `WarmupCountdownStartTime` |
| `+0x12d8` | `WarmupCountdownEndTime` |
| `+0x12dc` | `AircraftStartTime` |
| `+0x12e0` | `SafeZonesStartTime` |
| `+0x1c98` | `MapInfo` |
| `+0x1cb0` | `GamePhase` |
| `+0x1eb8` mask 1 | `bGameModeWillSkipAircraft` |
| `+0x1ee0` / `+0x1ee8` | `Aircrafts` array / count |
| `+0x1ef0` mask 1 | `bAircraftIsLocked` |

`UFortPlaylistAthena`: `+0x5c` `MaxTeamSize`, `+0x439` `bAllowBackfill`,
`+0x660` `bSkipWarmup`, `+0x664` `WarmupEarlyRequiredPlayerPercent`,
`+0x668` `AirCraftBehavior`, `+0x8c0` `SafeZoneStartUp`.

Enums: `EAirCraftBehavior` Default 0, OpposingAirCraftForEachTeam 1,
FlyTowardFirstCircleCenter 2, NoAircraft 3. `ESafeZoneStartUp`
UseDefaultGameBehavior 0, StartsWithWarmUp 1, StartsWithAirCraft 2,
StartsWithNoAirCraft 3.

`AFortAthenaAircraft`: `+0x260`, `+0x270`, `+0x280` are three multicast
delegates (entered drop zone, exited drop zone, flight ended; not reflected),
`+0x2a8` `FlightInfo` (0x28 bytes), `+0x2d0` `FlightStartTime`, `+0x2d4`
`FlightEndTime`, `+0x2d8` `DropStartTime`, `+0x2dc` `DropEndTime`, `+0x2e4`
`ReplicatedFlightTimestamp`, `+0x2e8` `FlightElapsedTime`, `+0x438`
`AircraftIndex`.

`AFortAthenaMapInfo`: `+0x3b0` `AircraftClass`, `+0x428` `FlightInfos`
(array, 0x28 bytes per element).

`AFortPlayerController` byte `+0x658`: `bFailedToRespawn` 1,
`bIsDisconnecting` 2, `bIsBeingKicked` 4, `bHasInitiallySpawned` 8,
`bAssignedStartSpawn` 0x10, **`bReadyToStartMatch` 0x20**,
`bClientPawnIsLoaded` 0x40.

## 2. The per-frame driver, `0x141975090` (read)

Signature `(AFortGameModeAthena* this, float DeltaSeconds)`. It sits in the
game mode vtable at `0x145dfb1b8`, slot `+0x708`, so it is the actor `Tick`
override (inferred from the signature). Nothing else calls
`UpdateWarmupPhase`, so **the game mode must tick for the warmup countdown to
run**.

Order of work each frame:

1. Parent tick `0x141eb4580`. Return if the GameState is not
   `AFortGameStateAthena`. Call `0x141961aa0` (not read).
2. `Phase = GameState.GamePhase`; `Playlist = current playlist`.
3. If there is a playlist, and one of: Phase is Warmup and
   `SafeZoneStartUp == StartsWithWarmUp`; Phase is Aircraft and
   `SafeZoneStartUp` is `StartsWithWarmUp` or `StartsWithAirCraft`; Phase is
   SafeZones. Then run the safe-zone update `0x141978230(this, Delta)`
   (card R2) and count down `SpawnActorDataList` (0x20 bytes per entry: count
   at `+8`, timer at `+0xc`; when the timer reaches 0 call `0x141967e60`).
4. Phase is Warmup: `UpdateWarmupPhase` (section 4). Phase is SafeZones with
   no playlist: the safe-zone update.
5. Always: `0x141978a80(this, Delta)` (not read).
6. Phase Warmup, Aircraft or SafeZones: add Delta to the match time at
   `+0x10cc` and publish `MatchTime` and `GamePhase` to the crash context.
7. Phase Aircraft or SafeZones: walk `SupplyDropSpawnDataList` (0x50 bytes per
   entry); type byte `+0x40` selects `0x14197a750` (type 0) or `0x141977370`
   (type 1). Not read; these are the supply drops.
8. Virtual slots `+0xd28` and `+0xd30`, then `0x1419797b0` and
   `0x1419783c0(this, 1, 0, 0)` (not read), then every mutator gets its own
   tick (mutator vtable slot `+0x720`).

## 3. `StartWarmupPhase`, `0x141972c30` (read)

Returns bool. Steps:

1. GameState must be `AFortGameStateAthena`, else return 0.
2. If the playlist has `SafeZoneStartUp == StartsWithWarmUp`, call
   `IsSafeZoneAllowed(this, 0, 0)` (card R2). In the default solo playlist
   this is not the case; check the value in `D4-game-data.md` before porting.
3. If `GamePhase` is already Warmup, return 1 and do nothing else.
4. Call `0x141eacd80` (394 lines, not read; a parent-class or zone-setup
   call made once before warmup).
5. If bSkipWarmup: log `Skipping warmup phase` and **return 0**.
6. Store the world time (`World + 0x548`) in `+0x10a0`.
7. If bSkipAircraft is clear, pre-spawn the aircraft (`0x141967bd0`, section
   6.1). If it is set, write `GameState.bGameModeWillSkipAircraft = 1`.
8. Set the phase to Warmup through the GameState setter `0x1419ae750`
   (section 3.1) and log `Set AthenaGamePhase to [Warmup].` (the argument is
   lost in the decompile; inferred from the log line).
9. Copy the runtime option byte `UFortRuntimeOptions + 0xa4` into
   bPauseWarmup.
10. If the playlist's `WarmupEarlyRequiredPlayerPercent > 0`, copy it to
    `+0xf8c`.

### 3.1 The phase setter, `AFortGameStateAthena` `0x1419ae750` (read)

`(GameState, uint8 NewPhase)`:

1. Authority only (`Role == 3`). No-op when the phase does not change.
2. Writes `GamePhase`.
3. For SafeZones and EndGame it calls `0x14069ac00` with 3 or 4 (not read; a
   small notification).
4. Reads the net driver through `0x143f76f10` and sets its connection timeout
   (`+0x7c`). For the SafeZones phase it uses the config value
   `InGameConnectionTimeOutPVP` from section
   `/Script/FortniteGame.FortGameModeZone` when that is above zero.
5. Calls `0x14199ac00(GameState, OldPhase)`. This is the same function the
   client runs when the phase replicates (1,244 lines, not read here). It is
   what plays the client-side reaction to a phase change, and on the server
   it runs directly.
6. Calls GameState virtual slot `+0x5e8` (inferred: force a net update).

**For the port:** always change the phase through this function. Writing the
property directly, as the G6 stand-in does, skips steps 4 to 6.

## 4. `UpdateWarmupPhase`, `0x14197a400` (read)

`now` is GameState virtual slot `+0x630` (server world time, inferred).
`end` is `GameState.WarmupCountdownEndTime`.

1. bPauseWarmup set: return.
2. `end < 0` (the property's start value must be negative for this to work):
   write `WarmupCountdownStartTime = now` and
   `WarmupCountdownEndTime = now + WarmupCountdownDuration`, log
   `Warmup phase initiated`, return. **A stand-in that leaves the end time at
   0 never starts the countdown and instead falls into step 3 on the first
   tick.**
3. `end < now` (countdown over):
   - Every mutator is asked through vtable slot `+0xa88` with a bool that
     starts true; any mutator can clear it. If it ends false, do nothing this
     tick.
   - Log `Warmup countdown is over. Attempting to start aircraft phase.`
   - If `GetNumPlayers()` (slot `+0x6a0`) is below 1: log
     `All players disconnected during warmup phase. Restarting session.` and
     call `0x1419602e0`, which calls slot `+0xc58` and sets a timer of 5 s or
     a little more to restart.
   - Otherwise store the ready-player count (section 4.1, with argument 0) in
     `+0xd78` and call the warmup-end helper `0x141975ef0` (section 5).
4. `end > now` and `end - now > WarmupEarlyCountdownDuration`:
   - Call `0x14199e380(GameState)` (asks mutators through slot `+0x890`; the
     result is not used here).
   - `count` = ready-player count. If
     `count >= CeilToInt(EarlyPercent * MaxPlayerCount)`: mutators may replace
     the early duration (slot `+0xa78`), then
     `WarmupCountdownEndTime = now + WarmupEarlyCountdownDuration`, set the
     early-countdown bit, log `Player percent reached to start early
     countdown.`

### 4.1 The ready-player counter, `0x14193af00` (read)

`(this, bool bExcludeFlagged)`. Walks the world's controller list and counts
a controller when all of these hold:

- it is an `AFortPlayerControllerAthena`,
- it has a PlayerState and that PlayerState is not `bIsSpectator`,
- **its `bReadyToStartMatch` bit is set**,
- when `bExcludeFlagged` is true, the byte at controller `+0x2d69` is 0 (not
  named).

**Consequence:** a client that never reaches "ready to start match" on the
server is not counted. With one test client and `RequiredPlayers=1`, the
early countdown only starts after that bit is set. This is the server half of
the readiness step in `G6_MAPPING_APPROACH.md`; find who sets the bit
(`ServerReadyToStartMatch`) before testing the countdown.

## 5. The warmup-end helper, `0x141975ef0` (read)

1. Call `StartAircraftPhase(this, 0)`. If it returns true, done. It returns
   true on normal completion or when the phase is already Aircraft, and
   false when there is no GameState, bSkipAircraft is set, the phase is
   already past Aircraft, the playlist is null, or an aircraft could not be
   spawned (read from the assembly, card R5 section 0).
2. Otherwise, if the playlist has `SafeZoneStartUp == StartsWithNoAirCraft`,
   or `0x14199e430(GameState)` is true (not read):
   - call `IsSafeZoneAllowed(this, 1)`; if it returns 0 and there is a
     MapInfo, set the phase through the setter (argument shown as 0);
   - for every `AFortPlayerControllerAthena` for which `PlayerCanRestart`
     (`0x14470a4e0`, the Blueprint-callable wrapper) is true, call
     `RestartPlayer` (game mode slot `+0x778`).
3. Otherwise set the phase and log `Set AthenaGamePhase to [None].`

## 6. `StartAircraftPhase`, `0x14196b640` (read)

`(this, bool bSecond)`. The second argument is 0 from the normal path and 1
from `0x141950090` (section 9). When it is 1 the aircraft block in step 4 is
skipped.

1. Looks up an online-subsystem interface (`0x140fc7960`, then slot `+0x2d8`
   with the engine name `GameSession`). On the first call (`+0xf80 == 0`),
   if that interface or the playlist is missing, or the session flag
   `+0x11` and the playlist's `bAllowBackfill` are both clear: set
   `+0xf80 = 1` and run `RemoveMissingPlayersFromSession` (`0x14195eb70`,
   777 lines, session bookkeeping, not read). Then `0x141977980` (recent
   players, not read). **Neither is seen by the client; leave both out and
   say so in a comment.**
2. Record the wall-clock start time in `+0x1090` and set `+0x1098`.
3. Early exits, all through the common exit:
   - no GameState or bSkipAircraft: log `Skipping aircraft phase`;
   - `GamePhase > Aircraft`: log `Already past aircraft phase`;
   - otherwise log `Initiating aircraft phase`; if `GamePhase == Aircraft`
     exit; if the playlist is null log `Playlist is Null!` and exit.
4. When `bSecond == 0` and `AirCraftBehavior != NoAircraft`:
   - `Count` = `GameState.TeamCount` when the behavior is
     `OpposingAirCraftForEachTeam`, else 1.
   - `now` = GameState time.
   - For `i` in `0..Count-1`: take `this->Aircrafts[i]`, or spawn one
     (section 6.1) when the slot is missing or null. On failure log
     `Unable to spawn aircraft.` and exit.
   - Add it to the GameState (`0x1419afc30`): appends to
     `GameState.Aircrafts`; on the first append it also sets
     `bAircraftIsLocked` and zeroes the int at GameState `+0x1418`.
   - Unless bPauseAircraft: start the flight (section 6.2) with `now`.
   - Bind three delegates on the aircraft:
     `+0x260` to `OnAircraftEnteredDropZone` (`0x1419520f0`),
     `+0x270` to `OnAircraftExitedDropZone` (`0x141952190`),
     `+0x280` to `OnAircraftFlightEnded` (`0x141952500`).
   - After the loop: `GameState.AircraftStartTime = now`.
   - Build an analytics record (max players, players at start, early flag,
     seconds spent in warmup) and send it through a stub. Leave out.
   - Call `0x14196d920` (named `StartMatchAbandonProcess` by a log string,
     924 lines, not read).
5. If the playlist's `MaxTeamSize > 1` and there is a game session: build a
   map of party members from the session. Session bookkeeping; leave out for
   solo.
6. For every `AFortPlayerControllerAthena` in the world (read, important):
   - zero the two 8-byte fields at controller `+0x7f8` and `+0x808`
     (`LastDamager`, `LastFallInstigator`);
   - call `0x142377ac0(PC)` (not read);
   - if the pawn is an Athena player pawn: `0x142314fe0(Pawn, 0)`,
     `0x14234d960(Pawn, 0)`, pawn slot `+0x1010` with 1, then broadcast the
     pawn's `OnEnteredAircraft` delegate (`+0x21a8`);
   - `0x142a3baf0(PC, 0, 0, 0)` (not read);
   - **destroy the pawn** (`0x143f6c200(Pawn, 0)`, the same function used to
     destroy the aircraft in section 8);
   - controller slot `+0x5d0`, then `0x1447ba3f0(PC, NAME_Spectating)`: the
     controller is put in the **Spectating** state (engine name 322).
7. If the runtime option byte `+0x837` is set, tell the game session
   `StartAircraft`. Then game session slot `+0x7e8` (joins closed), unless
   the session interface flag from step 1 is set.
8. If the playlist has `SafeZoneStartUp == StartsWithAirCraft`, call
   `IsSafeZoneAllowed(this, 0)`.
9. Set the phase to Aircraft through the setter; call `0x141975490`
   (section 6.3); log `Set AthenaGamePhase to [Aircraft].`
10. Set a 60.0 s timer on handle `+0x14e0`. The callback object has vtable
    `0x1459a9ed8`; **which function it calls is not read yet.**

**What this means for the design of the jump.** The server does not move the
warmup pawn onto the bus. It destroys every warmup pawn and puts every
controller into Spectating. The phase setter in step 9 then runs the
phase-change handler, which calls `PlacePlayersInAircraft` and so
`EnterAircraft` on each controller's aircraft component (card R5 sections 3
and 4). The player gets a new pawn when the game mode
calls `RestartPlayer` on the controller: that is exactly what the forced exit
does (section 7.2), and `ChoosePlayerStart` returns the aircraft start during
the Aircraft phase (`D2-match-flow.md` section 10, step 5). So the
compiled-out `ServerAttemptAircraftJump` almost certainly ends in the same
`RestartPlayer` call. Design the jump as: validate (phase is Aircraft, the
aircraft component says the player is on the bus, the bus is unlocked), then
`RestartPlayer(PC)`.

### 6.1 Spawning an aircraft, `0x1418d4da0` and `0x141967bd0` (read)

`0x141967bd0(this)` runs in `StartWarmupPhase`: `Count` as in step 4 above
(nothing when the behavior is `NoAircraft`); for each index it calls
`0x1418d4da0(World, i)` and appends the result to `this->Aircrafts`. So the
aircraft actors exist from the start of warmup, and the aircraft phase only
re-uses them.

`0x1418d4da0(World, Index)`:

1. Needs `World->GameState` to be `AFortGameStateAthena` with a `MapInfo`
   that passes `0x1418f1550` (not read; a validity check).
2. Spawns `MapInfo.AircraftClass` (must derive from the aircraft base class)
   with spawn parameters flags `|= 6` and collision override 1.
3. Sets `AircraftIndex = Index`, sets `FlightStartTime`, `FlightEndTime`,
   `DropStartTime`, `DropEndTime` to -1.0.
4. Copies `MapInfo.FlightInfos[Index]` (0x28 bytes) into the aircraft's
   `FlightInfo`.
5. Moves the actor to the flight start: location is the first 12 bytes of
   `FlightInfo`, rotation is made from the vector at `FlightInfo + 0xc`.

**Who fills `MapInfo.FlightInfos` (found, bodies not read).** The game mode
function named `OnPlaylistDataLoaded` by its log strings (it contains
`0x141955610`; that address is a chunk in the middle, the real start is one
of `0x141955570`, `0x141955500`, `0x141955490`) does this once:

- if `bFlightPathInitialized` (`+0x10c9`) is clear and the MapInfo passes
  `0x1418f1550`: call MapInfo `0x1418ef640` (994 lines), which calls
  `InitializeFlightPath` `0x1418e4610` (599 lines; log strings
  `InitializeFlightPath: Failed to project path onto spawn zone / drop zone
  volume / drop zone box` and `AircraftFlightPath,%.3f,%.3f,%.3f`), then set
  the bit;
- the same function also picks the storm centers (card R2 section 10).

It is the game mode virtual at slot `+0xd10` (start `0x141955570`), called
from `AFortGameStateAthena::OnPlaylistFullLoadCompleted`; the full trigger
chain from `OnRep_CurrentPlaylistInfo` is in card R6 section 1.
**The stand-in server must run this function (or its two calls) before
`StartWarmupPhase`, otherwise the aircraft spawns with an empty path.** The
flight path is projected onto map volumes (spawn zone, drop zone), so it
also needs those volumes from the real map; on `FortniteEmptyDedicated` it
will log one of the three failures above.

### 6.2 Starting the flight, `0x1418d5590` (read)

`(Aircraft, float Time)`. Only when `FlightStartTime < 0`:

- `FlightStartTime = Time`, `ReplicatedFlightTimestamp = Time`,
  `FlightElapsedTime = 0`;
- `FlightEndTime = Time + FlightInfo[+0x1c]`,
  `DropStartTime = Time + FlightInfo[+0x20]`,
  `DropEndTime = Time + FlightInfo[+0x24]`.

Otherwise it logs `StartFlightPath: Flight was already started.` The three
floats at `FlightInfo + 0x1c`, `+0x20`, `+0x24` are therefore the time to the
end of the flight, to the drop zone start and to the drop zone end.

### 6.3 `0x141975490` (read)

Resets every entry of `BuildingGameplayActorSpawnData` (5 ints per entry) and
rolls a new count and time for it from scalable floats in a GameState array
at `+0x2b8` (0x150 bytes per element), evaluated at the current
`SafeZonePhase`. Not seen by the client directly. Port later with the supply
drops.

## 7. The three aircraft delegates (read)

### 7.1 `OnAircraftEnteredDropZone`, `0x1419520f0`

If every aircraft in `GameState.Aircrafts` has the byte at aircraft `+0x2f0`
set, call the lock setter `0x1419adfe0(GameState, false)` (read). When it
unlocks a locked GameState it:

- stores `PlayersLeft + PlayerBotsLeft` in the int at GameState `+0x1418`;
- for every Athena controller with an Athena PlayerState, writes the current
  time into PlayerState `+0x1298` (not named);
- copies `PlayersLeft` and `TeamsLeft` into `BroadcastSpectatorInfo`
  (`+0x218`, `+0x21c`) when that object exists;
- writes `bAircraftIsLocked = false`.

`bAircraftIsLocked` is replicated, so this is the moment the client allows
the jump.

### 7.2 `OnAircraftExitedDropZone`, `0x141952190`

`(this, Aircraft)`. For every `AFortPlayerControllerAthena` whose team's
aircraft (`GetAircraftIndexFromTeam` `0x141990ec0`, then `GetAircraft`) is
this one, take the controller's aircraft component (`0x141695900`):

- component byte `+0xf8` set (the player is still on the bus):
  `RestartPlayer(PC)` (slot `+0x778`). This is the forced exit.
- byte `+0xf8` clear, and either byte `+0xf9` clear or the controller is not
  `bReadyToStartMatch`: log `Player [%s] failed to load before aircraft
  phase ended. Kicking player.` and kick through the game session (slot
  `+0x698`) with `Failed to load before game started.`

Then increment `+0xd90`. When it reaches `GameState.Aircrafts.Num()`:

- playlist `SafeZoneStartUp` is `StartsWithWarmUp` or `StartsWithAirCraft`:
  `StartSafeZonesPhase(this, 0)`;
- otherwise `IsSafeZoneAllowed(this, 0, 1)`, which itself ends in
  `StartSafeZonesPhase` (card R2).

The bytes `+0xf8` and `+0xf9` lie inside the aircraft component after its
reflected properties. From `EnterAircraft` (card R5 section 4.2): `+0xf8` is
"in the aircraft now" and `+0xf9` is "has already left the aircraft".

### 7.3 `OnAircraftFlightEnded`, `0x141952500`

`(this, Aircraft)`. Removes this game mode's three bindings from the aircraft
and destroys the aircraft.

## 8. `GetAircraft`, `AFortGameStateAthena` `0x141990d90` (read)

`(GameState, int Index)`. In Setup or Warmup it returns `Aircrafts[Index]`
or null without logging. In later phases a bad index logs `Aircraft Index is
invalid. This might be ok.` and a null entry logs `Aircrafts[%d] is not
valid.` unless the world is a certain kind (`0x144693a30`).

## 9. The other caller, `0x141950090` (read)

A second entry that jumps the match forward (inferred to be a cheat or an
admin command; find the caller before relying on it):

- Phase Warmup: store the ready-player count, `StartAircraftPhase(this, 1)`.
- Phase Aircraft: destroy every aircraft.
- Then `IsSafeZoneAllowed(this, 0, 1)`, and `RestartPlayer` for every Athena
  controller without a pawn that may restart, followed by a mutator
  notification (slot `+0x728`).

Useful as a test hook once ported: it reaches the storm without the bus.

## 10. Port order and what to check in one run

Port in this order; each item names the property the client watches.

| Step | Function | Client-visible result |
|---|---|---|
| 1 | phase setter `0x1419ae750` | `GamePhase` replicates and the phase-change handler runs |
| 2 | `StartWarmupPhase`, pre-spawn aircraft | `Aircrafts` actors exist, phase Warmup |
| 3 | tick driver, `UpdateWarmupPhase`, counter | `WarmupCountdownStartTime` and `EndTime` replicate; HUD countdown |
| 4 | warmup-end helper, `StartAircraftPhase`, flight start | `GameState.Aircrafts`, `AircraftStartTime`, aircraft flight times, phase Aircraft |
| 5 | three delegates | unlock, forced exit, aircraft destroyed |

One probe run should print, once per second: `GamePhase`, both warmup times,
the ready-player count, `bReadyToStartMatch` per controller, the number of
entries in the game mode and GameState `Aircrafts` arrays, each aircraft's
four flight times, `bAircraftIsLocked`, `+0xd90`, and per controller the pawn
pointer and the controller state name. Every condition in this card reads one
of those values.

## 11. Not read yet (addresses for the next pass)

The phase-change handler, the flight path setup, the `StartAircraftPhase`
return value and the aircraft component flags are now in card R5.

| Address | What | Needed for |
|---|---|---|
| vtable `0x1459a9ed8` | the 60 s timer callback set at aircraft start | unknown effect |
| `OnPlaylistDataLoaded` `0x141955570`, MapInfo `0x1418ef640`, `InitializeFlightPath` `0x1418e4610` | **done**: bodies in R5 section 1, trigger in R6 section 1 | aircraft path |
| `0x141eacd80` | pre-warmup call | unknown |
| `0x141961aa0`, `0x141978a80`, `0x1419783c0` | **done**: storm inside check and placement update in R6 section 5, supply drop scheduler in R6 section 4 | G8 storm damage |
| `0x1419797b0` | other per-frame work, not read | unknown |
| `0x14196d920` | `StartMatchAbandonProcess` | probably not needed |
| `0x14199e430` | GameState test used when the aircraft is skipped | skip path |

## 12. Match start: additions to `D2-match-flow.md` sections 2 and 3 (read)

`InitGame` (`0x14194cd90`) and `HandleMatchHasStarted` (`0x14194bb50`) were
re-read; D2 is accurate. Points it does not state:

- `InitGame` calls its parent first (`0x141ea1630`). Every URL option is
  also accepted as a command-line switch of the same name.
- The alternate defaults (countdown 15.0 s, required players 1, and
  `numplayers` from the command line) are taken when the world test
  `0x1446939f0` or `0x141f3ad70` is true. Which kind of world that is has
  not been established; do not rely on it, pass `RequiredPlayers=1`.
- `HandleMatchHasStarted` computes bPauseWarmup from a GameState query
  (`0x1419b0c20`) just before warmup, so the URL option can be overridden
  there.
- It calls `StartWarmupPhase`; when that returns 0 it calls the warmup-end
  helper (section 5). Then it spawns the MegaStormManager from
  `MegaStormManagerClass` and stores it at `+0x1018`.
- Players enter `AlivePlayers` through `AddToAlivePlayers` `0x141933cb0`,
  called from `PostLogin` and `FinishRestartPlayer` (card R3 section 7).
