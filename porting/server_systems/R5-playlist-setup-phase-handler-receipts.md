# R5: flight path and storm-center setup, the phase-change handler, bus placement, client receipts

Date: 2026-11-12. This card closes the items left open by R1 to R4. Same
conventions and confidence labels (**read**, **inferred**, **not read**) as
`R1-warmup-and-aircraft-code.md`. The three largest functions here are 600
to 1,900 lines of decompile, most of it inlined vector math and the inlined
table cache (R2 section 1.1). They were read for structure: inputs, outputs,
branches, calls and log lines. Where a formula was not followed term by
term, the text says so.

## 0. Corrections to R1 and to the note added to `G6_MAPPING_APPROACH.md`

1. **`PlacePlayersInAircraft` does run on the server.** It is not called by
   `StartAircraftPhase` directly, but the phase setter calls the
   phase-change handler, and that handler calls it when the new phase is
   Aircraft (section 3). R1 section 6 and the map note said the server only
   destroys pawns and sets Spectating; both things happen, in that order:
   pawns destroyed and state Spectating in `StartAircraftPhase` step 6, then
   placement on the bus in step 9 through the setter.
2. **`StartAircraftPhase` return value (R1 section 5), now established** from
   the assembly (the return register is `dil`, cleared at `0x14196c45e`, set
   at `0x14196b86e` and `0x14196c3f1`):
   - returns **true** when the phase is already Aircraft, and on normal
     completion;
   - returns **false** when there is no GameState, bSkipAircraft is set, the
     phase is already past Aircraft, the playlist is null, or an aircraft
     could not be spawned.
   So the warmup-end helper takes its "no aircraft" branch exactly in those
   five cases.
3. **The two aircraft component flags (R1 section 7.2) are identified**
   (section 4): `+0xf8` is "in the aircraft now", `+0xf9` is "has already
   left the aircraft".

## 1. Flight path setup

### 1.1 MapInfo `0x1418ef640` (read for structure; 994 lines)

`(MapInfo, GameState, bool bForceFirstCircle, float, float, float)`. Called
from the game mode's `OnPlaylistDataLoaded` (R1 section 6.1) with
`(MapInfo, GameState, 0, 0, …)`, and from four mutators.

Names: MapInfo `+0x3b8` `AircraftSpawnZone` (box), `+0x3cc`
`AircraftDropZone` (box), `+0x3e0` `AircraftDropVolume`, `+0x428`
`FlightInfos`, `+0x458` `AircraftDeviationAngle`, `+0x478`
`AircraftDistanceFromMidLine`, `+0x498` `AircraftHeight` (all three scalable
floats). GameState `+0x13b8` `TeamFlightPaths`, `+0x13c8`
`FlightPathMidLine`, `+0x13f0` `DropZoneCenter`, `+0x13f8`
`FlightPathSelectionCenter`, `+0x14a0` `AirCraftBehavior`. Playlist `+0x669`
`bUseCustomAircraftPathSelection`, `+0x66b`
`bAircraftDropOnlyWithinSafeZone`, `+0x670` `AircraftSafetyMarginPct`.

Steps:

1. Read the playlist and the game mode (the pointer at GameState `+0x220`,
   cast to the Athena game mode; it is needed for `SafeZoneLocations`).
2. Take the spawn zone and drop zone boxes, offset by the MapInfo actor's
   location, and `AircraftHeight`.
3. When `AirCraftBehavior == OpposingAirCraftForEachTeam`, also read
   `AircraftDistanceFromMidLine`.
4. Empty `GameState.TeamFlightPaths`.
5. Pick the point the path must pass through:
   - `AirCraftBehavior == FlyTowardFirstCircleCenter`, or
     `bForceFirstCircle`: make sure the storm centers exist (game mode
     `0x14194de20`, which runs section 2 when
     `bSafeZoneLocationsInitialized` is clear) and use the first circle.
     With fewer than 3 storm phases defined it logs `InitializeFlightPath:
     Can't properly align the bus on the first safe zone as we are missing
     safezone definition! Playlist=[%s].`;
   - otherwise the drop zone: the bounds of `AircraftDropVolume` when it
     exists (and the behavior is not the per-team one), else the
     `AircraftDropZone` box.
6. Store the drop zone center in the GameState (`DropZoneCenter`).
7. Pick a random direction: several `rand()` calls give an angle and an
   offset inside the box; `AircraftDeviationAngle` gives a random deviation.
   A custom-selection branch (cvar `0x1478c2418` plus playlist
   `bUseCustomAircraftPathSelection`) uses the three float arguments
   instead. **The exact formulas were not followed term by term.**
8. Empty `MapInfo.FlightInfos`. For each aircraft (1, or `TeamCount` for the
   per-team behavior; odd-numbered aircraft get the mirrored side): call
   `InitializeFlightPath` (section 1.2) and append the result to
   `FlightInfos` and to `GameState.TeamFlightPaths`.
9. With fewer than 2 aircraft, call `InitializeFlightPath` once more with
   offset 0 for the mid line, and write `GameState.FlightPathMidLine` and
   `FlightPathSelectionCenter` (the midpoint of the path).

### 1.2 `InitializeFlightPath`, `0x1418e4610` (read for structure; 599 lines)

`(MapInfo, FAircraftFlightInfo* Out, GameState, PointA, PointB, Center,
float Height, SpawnBox*, DropBox*, float SideOffset, float AngleDegrees)`.

`FAircraftFlightInfo` (0x28 bytes): `FlightStartLocation` (12 bytes),
`FlightStartRotation` (12 bytes), `FlightSpeed` (`+0x18`),
`TimeTillFlightEnd` (`+0x1c`), `TimeTillDropStart` (`+0x20`),
`TimeTillDropEnd` (`+0x24`). The last three start at -1.0. (Names of the
three times are from how `0x1418d5590` uses them, R1 section 6.2.)

1. Zero the output, set the three times to -1.
2. `SideOffset != 0`: move the line sideways by that distance
   (perpendicular to A→B). `AngleDegrees != 0`: rotate it around `Center`.
3. Project the line onto the spawn zone box (`0x1418f5b60`); failure logs
   `Failed to project path onto spawn zone.` A cvar (`0x1478c241c`) with the
   per-team behavior uses a different start computation instead.
4. Project the line onto the drop zone: `0x1418f5e10` with
   `AircraftDropVolume` when the MapInfo has one (failure: `Failed to
   project path onto drop zone volume.`), else `0x1418f5b60` with the drop
   box (failure: `Failed to project path onto drop zone box.`).
5. If the playlist has `bAircraftDropOnlyWithinSafeZone` and the behavior is
   `FlyTowardFirstCircleCenter`: clip the drop segment to the first storm
   circle, with the radius reduced by `AircraftSafetyMarginPct` (not below
   200 units).
6. `0x1418f8580(MapInfo, Start, …, DropStart, DropEnd, Out, Height)` (not
   read) fills the struct: start location and rotation, speed and the three
   times.
7. When the runtime option byte `+0x837` is set, send
   `AircraftFlightPath,%.3f,%.3f,%.3f` (the three times) to the game
   session.

**What the port needs from the map:** `AircraftSpawnZone`,
`AircraftDropZone` (two boxes stored in the MapInfo actor's properties, so
they come with the MapInfo Blueprint, not with terrain) and optionally
`AircraftDropVolume` (an actor reference, which does need the level). On a
world with a MapInfo that has the two boxes, the path can be computed
without the terrain.

## 2. Storm-center setup

### 2.1 MapInfo `0x1418e7080` (read for structure; 1,915 lines)

`(MapInfo, TArray<FVector>* Locations, RejectVolumes*, FVector* Root,
int StartPhase, bool bCenterAll)`. Called through the wrapper
`0x1418e7000` from `OnPlaylistDataLoaded` with
`(GameState.MapInfo, &GameMode.SafeZoneLocations, …)`, and from the
re-center function `0x14194dce0` with `(…, &Point, SafeZonePhase, flag)`.

1. `Count` = number of storm phases (`0x1418edcb0`). Grow `Locations` to
   `Count` zeroed vectors.
2. `Locations[0] = *Root`; when the world has an Athena GameState, the X and
   Y of element 0 are overwritten with
   `GameState.FlightPathSelectionCenter`. **So the first circle is centered
   on the middle of the bus path, which is why the flight path must be set
   up first** (that is the order inside `OnPlaylistDataLoaded`).
3. For each later phase `i` (from `StartPhase + 1`): fill a parameter block
   from the tables at level `i` and from the previous center, then
   - normally: `Locations[i] = PickSafeZoneLocation(MapInfo, &Block,
     RejectVolumes)`;
   - when `bCenterAll` is set and the phase's radius is not positive:
     `Locations[i] = *Root`.

   Which table value goes to which field of the block was matched by the
   log line in section 2.2, not traced through the 1,915 lines.

### 2.2 `PickSafeZoneLocation`, `0x1418f4530` (read; 305 lines)

`(MapInfo, FVector* Out, Block*, RejectVolumes*)`. Block fields, from the
reads and the log line `PickSafeZoneLocation[%d]: Radius: … ForceDistanceMin:
… ForceDistanceMax: … RejectRadius: … RejectOuterDistance: … MaxRadius: …`:
`+0x00` index, `+0x04` root location, `+0x14` max radius, `+0x18`
`ForceDistanceMin`, `+0x1c` `ForceDistanceMax`, `+0x20` `RejectRadius`,
`+0x24` `RejectOuterDistance`, `+0x28` previous center, `+0x38` a bool.

1. "Forced distance" mode when `ForceDistanceMax > 0` and
   `ForceDistanceMax >= ForceDistanceMin`.
2. Otherwise, when `RejectOuterDistance >= 0`: pick radius = max radius −
   `RejectOuterDistance` (logged). If that is not above `RejectRadius`, log
   `RejectOuterDistance will make acceptable radius too small. Centering.`
   and use 0.
3. Max radius `<= 0`: log `Invalid max radius. Returning root location.` and
   return the root.
4. Loop, at most 10,000 tries:
   - forced mode: a random direction (rejection-sampled inside the unit
     disc) at a distance between `ForceDistanceMin` and `ForceDistanceMax`
     from the previous center;
   - per-team bus behavior with 2 teams: a random distance along the
     GameState's flight path mid line direction (`+0x13d4`), on a random
     side;
   - otherwise: a random point in a disc of the pick radius
     (`0x142c5c970`).
   The candidate is rejected when it falls in one of the reject rectangles
   (`RejectVolumes`, four floats each), when it is farther than the allowed
   radius from the previous center, when `SafeZoneVolumeDefinitions` (MapInfo
   `+0x538`, 0x28 bytes each: a volume plus a scalable-float probability)
   exclude it, or when it is closer than `RejectRadius` to the previous
   center (not in forced mode).
5. On success log `Setting Location: %0.2f, %0.2f, %0.2f`. After 10,000
   failures log `Failed to find valid location using rejection.` and fall
   back to a point computed from the previous center and the root.

`rand()` is the C runtime generator, so the circles differ every match
unless the server seeds it.

## 3. The phase-change handler, GameState `0x14199ac00` (read for structure; 1,244 lines)

`(GameState, uint8 OldPhase)`. This is `OnRep_GamePhase`; the phase setter
calls it directly on the server (R1 section 3.1). In order:

1. Return if the phase did not change. Publish `GamePhase: %s` to the crash
   context.
2. New phase Warmup: bump a global counter (analytics stub).
3. Refresh the UI state tag: GameState slot `+0x920`, then `0x141eaee70`.
   This is the client-side tag from `G6_MAPPING_APPROACH.md` step 5.
4. New phase Setup, Warmup or Aircraft: walk the world's level-streaming
   objects and change which are requested (the warmup-island sublevels;
   details not read).
5. Authority and new phase Aircraft: a second streaming walk (inferred:
   release the warmup island).
6. New phase Warmup:
   - authority, and the playlist test `0x141a4a530` is true: call
     `0x1419fc560` for every Athena controller (not read);
   - authority: a second loop over controllers (body not read);
   - push the `LobbySoundMix` (client audio).
7. Old phase Warmup (leaving warmup): the same playlist test and loop on
   the authority, pop the sound mix, and on the authority
   `0x1419ae490(GameState)` (not read).
8. **New phase Aircraft:**
   - non-authority only: `0x1419a9580(GameState)`;
   - `0x141988940(GameState)`;
   - **`PlacePlayersInAircraft(GameState)`** (section 4), on server and
     client.
9. Authority and new phase Warmup: `UtcTimeStartedMatch = now`.
10. New phase EndGame with a replay driver: on the authority, when the
    runtime option byte `+0xbbb` is set, `0x1419b59e0`.
11. `0x141e9a1a0`, GameState slot `+0xa70`, `0x141eb30f0` (not read).
12. Leaving Setup or Warmup for a later phase, when `0x14197e350` is true:
    one more call (not read).
13. Broadcast the `GamePhaseChanged` delegate (`+0x1240`) with the new
    phase. Blueprints listen to this.

**For the port:** steps 8, 9 and 13 are the ones with server effects the
client depends on. Steps 4 to 7 are mostly client presentation; on the
server the level-streaming walks matter only once the real map is loaded.

## 4. Bus placement

### 4.1 `PlacePlayersInAircraft`, GameState `0x1419a8ec0` (read)

1. Every mutator may take over through slot `+0xb40`; if one returns true,
   return.
2. `Aircrafts` empty: log `early out. Aircrafts TArray is empty.` and
   return. Any null entry: log `early out. Aircrafts TArray has invalid
   entries. Possibly still replicating.` and return.
3. For every `AFortPlayerControllerAthena`, and for each aircraft index
   `i`:
   - the controller's index is 0, or with the per-team behavior
     `Team - Playlist.DefaultFirstTeam` (in Setup and Warmup an invalid
     index is replaced by 0 with a log line);
   - if the index is `i`, the controller has an aircraft component and both
     its flags `+0xf8` and `+0xf9` are clear: **`EnterAircraft(Component,
     GetAircraft(i))`** (section 4.2);
   - then `GetAircraft(i)` and `0x1419f5840` (arguments lost in the
     decompile; inferred to point the controller's view at the aircraft).
4. On the authority, for each controller with the object at `+0x1a40`:
   `0x1424a51b0(Object, 1)`, `(Object, 3)`, `(Object, 4)` (not named;
   inferred quest or stat triggers).

### 4.2 `EnterAircraft`, aircraft component `0x141ccfa40` (read in part; 458 lines)

`(UFortControllerComponent_Aircraft*, AFortAthenaAircraft*)`.

- If `+0xf9` is set, log `EnterAircraft: [%s] is attempting to enter
  aircraft after having already exited.` (it continues).
- If `+0xf8` is set, return: already in.
- For an Athena controller: `0x14210e280(PC)`, `0x142123bf0(PC, 1)`, and
  take a reference from the controller (`+0x3128`) when its HUD-side object
  exists. Log `EnterAircraft: %s`.
- Then controller virtual calls (`+0xbd8`, `+0xbe0`) and the rest of the
  function, **not read**: this is where `+0xf8` is set and
  `CurrentAircraft` (`+0xc0`) is written.

So the flags mean: `+0xf8` in the aircraft now; `+0xf9` has left the
aircraft. That makes the forced exit in R1 section 7.2 read as: still in
the aircraft → `RestartPlayer`; not in it and (never left it, or not ready)
→ kicked for failing to load.

**Design consequence for the jump re-creation** (`ServerAttemptAircraftJump`
is compiled out): validate that the phase is Aircraft, `bAircraftIsLocked`
is false and the component has `+0xf8` set; then do what the forced exit
does, `RestartPlayer(PC)`; the component must end with `+0xf8` clear and
`+0xf9` set. Find the component's exit function (the counterpart of
`EnterAircraft`; look for the writer of `+0xf9`) and call it rather than
writing the flags by hand.

## 5. Client receipts (read)

These run on the client. The server only has to call them with correct
arguments.

- **Winner.** `StartEndGamePhase` (R3 section 4 step 5) calls two event
  thunks on the winner's controller:
  - `0x142a52de0(PC, FinisherPawn, FinishingWeapon, DeathCause)`. By its
    position among the Athena controller's generated thunks and its
    arguments this is **`ClientNotifyWon`** (inferred; the exec side is
    `0x142a572e0`, the client body `0x1419c6120`).
  - `0x142a3bda0(PC)`: a no-argument event on `AFortPlayerController`, not
    named.
  - Before them, `0x1419f4520(PC, 1)` passes 1 to the PlayerState function
    `0x141a5aa60` (inferred: place = 1).
  So `ClientNotifyWon` is already sent by real code. The re-creation must
  not send it again.
- **`ClientOnPawnDied`** (`0x1423cd300`, `AFortPlayerControllerZone`; exec
  `0x142a6ff70`), argument `FFortPlayerDeathReport`. The body closes menus
  and input (`0x1423a8b00`, `0x142393980`, `0x142392550`), leaves a vehicle
  through the interaction component, drops the build preview, sets
  `PendingSpectatorLocation` to the pawn location plus 160 up, and copies
  the report field by field into the controller at `+0x2818`. The copy
  shows the report layout used on the wire: 3 ints, 2 pointers, 2 ints, 2
  pointers, a 2-bit flag word, a tag container at `+0x40`, a pointer at
  `+0x60`, then 4 ints. Total 0x78 bytes, matching D6.
- **`NetMulticast_Athena_BatchedDamageCues`** (`0x142334fc0`): on any
  machine that is not a dedicated server (`0x143f7a920() != 1`) it plays
  the cue (`0x142345360`) from the shared and non-shared structs, and a
  second time for the shield part when the shared data has one. On the
  dedicated server the body does nothing. **The server's job is only to
  call the multicast with filled structs**; who calls it (the damage path)
  is not in this function.

The remaining receipts (`ClientNotifyTeamWon`, `ClientSendMatchStatsForPlayer`,
`ClientSendTeamStatsForPlayer`, `ClientSendEndBattleRoyaleMatchForPlayer`,
`ClientSetDeathReport`) were not opened: nothing in the real server code
read so far calls them, so they belong to the re-created death and
placement sequence (D6 section 2), and their argument structs come from
reflection.

## 6. Order of server work at match start, as it now stands

1. `InitGame` (options).
2. `OnPlaylistDataLoaded`: flight path (section 1), then storm centers
   (section 2). Both need the MapInfo.
3. `HandleMatchHasStarted` → `StartWarmupPhase`: aircraft pre-spawned from
   `MapInfo.FlightInfos`, phase Warmup.
4. Players: `PostLogin` → `PickTeam` → `AddToAlivePlayers`;
   `ServerReadyToStartMatch` → `bReadyToStartMatch`.
5. Tick → `UpdateWarmupPhase` → warmup end → `StartAircraftPhase`: pawns
   destroyed, controllers Spectating, phase Aircraft → handler →
   `PlacePlayersInAircraft` → `EnterAircraft`.
6. Aircraft enters the drop zone → unlock. Jump (re-created) or forced exit
   → `RestartPlayer`.
7. All aircraft left the drop zone → `IsSafeZoneAllowed` →
   `StartSafeZonesPhase` → tick activates and advances the storm.
8. Removal → win check → `StartEndGamePhase` → end-match helper →
   `HandleMatchHasEnded`.

## 7. Still not read

| Address | What |
|---|---|
| `0x1418f8580` | fills `FAircraftFlightInfo` (speed and the three times) |
| `0x1418f5b60`, `0x1418f5e10` | line-onto-box and line-onto-volume projection |
| rest of `0x141ccfa40`, and the component's exit function | where `+0xf8` / `+0xf9` are written |
| `0x1419f5840`, `0x141988940`, `0x1419a9580`, `0x1419ae490`, `0x1419fc560` | called from the phase handler and placement |
| the random direction formulas in `0x1418ef640` | only needed for exact parity of bus paths |
| real start of `OnPlaylistDataLoaded` and who binds it | how the server triggers step 2 |
