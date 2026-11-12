# R2: the storm (safe zone), read line by line from the client

Date: 2026-11-12. Detailed read of the real server code for the storm part of
G7. Same conventions and confidence labels (**read**, **inferred**,
**not read**) as `R1-warmup-and-aircraft-code.md`. Storm numbers (wait and
shrink times, radii, damage) are in `D4-game-data.md` section 2; this card is
about the code that uses them.

## 0. Correction to `D2-match-flow.md` section 8

D2 calls indicator byte `+0x258` "bPaused". The reflected bit there is
**`bSafezoneEventDriven`** (mask 1). `bPaused` is `+0x259` and
`bPausedForPreview` is `+0x25a`. The code writes `bSafezoneEventDriven`, not
the pause flag (section 3).

## 1. Names

`AFortSafeZoneIndicator`: `+0x220` `LastRadius`, `+0x224` `NextRadius`,
`+0x228` `NextNextRadius`, `+0x22c` `LastCenter`, `+0x238` `NextCenter`,
`+0x244` `NextNextCenter`, `+0x250` `SafeZoneStartShrinkTime`, `+0x254`
`SafeZoneFinishShrinkTime`, `+0x258` `bSafezoneEventDriven`, `+0x259`
`bPaused`, `+0x25a` `bPausedForPreview`, `+0x264` (int, next-next mega storm
thickness, inferred), `+0x268` `NextMegaStormGridCellThickness`, `+0x26c`
`MegaStormDelayTimeBeforeDestruction`.

`AFortGameModeAthena`: `+0xf94` `bSafeZoneActive`, `+0xfb0`
`SafeZoneIndicatorClass`, `+0xfe0` `SafeZonePhase`, `+0xfe8`
`SafeZoneIndicator`, `+0xff0` / `+0xff8` `SafeZoneLocations` array / count
(12 bytes per element, a vector), `+0x1018` `MegaStormManager`, `+0xaa0`
`OnSafeZoneUpdated` delegate.

`AFortGameStateAthena`: `+0x11e8` `SafeZonePauseTime`, `+0x11ec`
`TotalFinalCountdownTime`, `+0x11f1` `bPlaylistStoppedSafeZonePhases`,
`+0x12d2` `bIsInCountdown`, `+0x12d3` `bIsInFinalCountdown`, `+0x12e0`
`SafeZonesStartTime`, `+0x1770` `SafeZoneIndicator`.

`AFortAthenaMapInfo`: `+0x4f8` `SafeZoneStartDelay` (scalable float),
`+0x518` `SafeZoneStartingRadius` (scalable float), `+0x548`
`SafeZoneDefinition`.

`UFortPlaylistAthena`: `+0x8c0` `SafeZoneStartUp`, `+0x8cc`
`LastSafeZoneIndex`.

### 1.1 The cached definition table (read)

`SafeZoneDefinition` is twelve scalable floats, 0x20 bytes each, starting at
MapInfo `+0x548`: `Count`, `Radius`, `ForceDistanceMin`, `ForceDistanceMax`,
`RejectRadius`, `RejectOuterDistance`, `WaitTime`, `ShrinkTime`, then
`MegaStormGridCellThickness`, `PlayerCapSolo`, `PlayerCapDuo`,
`PlayerCapSquad`. The values are instance data on the MapInfo actor placed
in `Apollo_Terrain.umap` (rows `Default.SafeZone.*` of `AthenaGameData`);
the rows, their keys and what a stand-in MapInfo must set are in card R6
section 2.

Every storm function starts with the same inlined block: if the int at
MapInfo `+0x6c8` differs from the global `0x148115c60`, it evaluates `Count`
into `+0x6cc` and fills eleven float arrays at `+0x6d0`, `+0x6e0`, … `+0x770`
with each curve's value at level 0, 1, … Count-1. So:

| Array | Holds |
|---|---|
| `+0x6d0` | `Radius[i]` |
| `+0x720` | `WaitTime[i]` |
| `+0x730` | `ShrinkTime[i]` |
| `+0x740` | mega storm grid cell thickness `[i]` |

The block is a cache and the global is its version stamp (inferred). **In the
port, write it once as a helper** (`GetSafeZoneValue(Curve, Phase)`) and read
the curves directly; the cache has no behavior of its own. The helper
`0x1418edcb0(MapInfo)` returns the number of phases (`Count`).

## 2. `IsSafeZoneAllowed`, `0x141971930` (read)

`(this, bool bForce, bool bStartPhase)`. Despite the name it also arms the
storm. Returns bool.

1. Needs an Athena GameState with a MapInfo, and more than 1 phase. Else 0.
2. bSkipSafeZones set: log `Skipping SafeZone phase: bSkipSafeZones == true`,
   return 0.
3. When `bForce` is false, return 1 **without arming anything** if the
   current phase already has the storm running:
   - `GamePhase == SafeZones`;
   - `GamePhase == Warmup` and the playlist says `StartsWithWarmUp`;
   - `GamePhase == Aircraft` and the playlist says `StartsWithWarmUp` or
     `StartsWithAirCraft`.
4. Otherwise arm it: `bSafeZoneActive = 0`,
   `GameState.SafeZonesStartTime = now + SafeZoneStartDelay`.
5. If `bStartPhase`, return `StartSafeZonesPhase(this, bForce)`. Else 1.

So for the default playlist the sequence is: all aircraft leave the drop zone
(R1 section 7.2), `IsSafeZoneAllowed(this, 0, 1)` sets the start time and
calls `StartSafeZonesPhase`, and the first circle appears when the per-frame
update sees `now > SafeZonesStartTime`.

## 3. `StartSafeZonesPhase`, `0x141971aa0` (read)

`(this, bool bForce)`. Returns bool.

1. Needs an Athena GameState with a MapInfo. bSkipSafeZones: log and
   return 0.
2. Skip the body when `GamePhase == SafeZones` and `bForce` is false.
3. Set the phase to SafeZones through the phase setter (R1 section 3.1;
   argument inferred from the log line `Set AthenaGamePhase to [SafeZones].`).
4. If `SafeZoneIndicator` already exists, for `P = SafeZonePhase`:
   - `WaitTime[P]` missing or `>= 0`: `bSafezoneEventDriven = 0`,
     `SafeZoneStartShrinkTime = now + WaitTime[P]` (0 when missing),
     `SafeZoneFinishShrinkTime = StartShrinkTime + ShrinkTime[P]` (0 when
     missing);
   - `WaitTime[P] < 0`: `bSafezoneEventDriven = 1` (inferred from the branch
     structure; the `else` was merged by the decompiler, confirm in the
     assembly). A negative wait time means the circle is advanced by an
     event, not by time.
5. If `0x14199e430(GameState)` is false, call a GameState function at
   `0x1419964d0` (the tool resolved this address to the wrong function start;
   **not read**).
6. Return 1.

## 4. The per-frame update, `0x141978230` (read)

Called from the tick driver (R1 section 2). `now` = GameState time.

1. If `bSafeZoneActive` is clear and `now > GameState.SafeZonesStartTime`:
   run the activation (section 5).
2. The rest runs only when `GameState.SafeZonePauseTime < 0` (not paused)
   and there is an indicator.
3. Phase Warmup or Aircraft (storm started early by the playlist): if
   `bPausedForPreview` is clear, `SafeZonePhase == 0`,
   `now >= SafeZoneFinishShrinkTime` and not event driven: advance (section
   6) with index -1. Then set `bPausedForPreview = 1`.
4. Other phases: clear `bPausedForPreview` if set. If
   `now > SafeZoneFinishShrinkTime` and not event driven, and
   `SafeZonePhase < Count - 1`:
   - if the playlist has `LastSafeZoneIndex >= 0` and it equals
     `SafeZonePhase`: stop here (`0x1419b0f50`, section 7);
   - otherwise advance with index -1.

## 5. Activation (`SpawnInitialSafeZone`, inferred name), `0x141968920` (read)

1. `bSafeZoneActive = 1`, `SafeZonePhase = 0`.
2. `0x1419635f0` (section 8), `0x1419634f0` (section 8), `0x141975490` (R1
   section 6.3), then every mutator through slot `+0x738`.
3. If there is no indicator: spawn `SafeZoneIndicatorClass` (spawn flags
   `|= 2`, collision override 1) and store it in `SafeZoneIndicator`.
4. `NextCenter = LastCenter = SafeZoneLocations[0]`.
   `GameState.SafeZoneIndicator = indicator`.
5. Start and finish shrink times exactly as section 3 step 4 with `P = 0`.
6. `LastRadius = NextRadius = max(SafeZoneStartingRadius, Radius[0])`.
7. `NextMegaStormGridCellThickness = RoundToInt(thickness[0])`;
   `MegaStormDelayTimeBeforeDestruction` copied from the MegaStormManager
   (`+0x22c`) when there is one.
8. `0x141964450(this, GameState, MapInfo)`: fills `NextNextCenter`,
   `NextNextRadius` and the int at `+0x264` from `SafeZoneLocations[P+1]`
   and the tables (368 lines; only the writes were read).
9. `0x141a5b1f0(indicator, LastRadius, &LastCenter)` (not read; applies the
   radius and center to the indicator), then `0x141961aa0`.

## 6. Advance, `0x14196ede0` (read; 1,548 lines, most of it the inlined cache)

`(this, int ForcedIndex)`. `-1` means "next phase".

1. Needs GameState and MapInfo. `0x1419635f0`.
2. `ForcedIndex < 0`: `SafeZonePhase += 1`. Otherwise
   `SafeZonePhase = ForcedIndex`.
3. `0x1419af220(GameState, phase)` (empty stub in the client), `0x1419634f0`,
   `0x141975490`.
4. Centers and radii.
   - Normal advance (`-1`): `LastCenter = NextCenter`;
     `NextCenter = SafeZoneLocations[SafeZonePhase]`;
     `LastRadius = 0x141a48240(indicator)` (read: returns `Radius`,
     indicator `+0x39c`); `NextRadius = Radius[SafeZonePhase]` (0 when out of range).
   - Forced index: `LastCenter = SafeZoneLocations[P]`;
     `NextCenter = SafeZoneLocations[P+1]` (or `[P]` when `P+1` is past the
     end); `LastRadius = Radius[P]`; `NextRadius = Radius[P+1]` (or
     `LastRadius` when past the end).
5. `NextMegaStormGridCellThickness = RoundToInt(thickness[P])`; then
   `0x141964450` for the next-next values.
6. Start and finish shrink times as section 3 step 4.
7. Mega storm: skipped with a log line when the thickness is below 1, the
   cvar is off, or there is no MegaStormManager; otherwise
   `0x1419fba60(MegaStormManager, …)` once per grid ring. Leave out on the
   first pass (D2 already says so).
8. Indicator slot `+0x5e8` (inferred: force net update).
9. For every Athena player pawn in the world: take an object through the
   pawn's interface at `+0x4c8` and call its slot `+0x5f0` with the 8 bytes
   at pawn `+0x1224` (not named; inferred to refresh a per-pawn storm
   effect).
10. Normal advance only: `0x1419ac150(GameState, &NextCenter, Radius[P])`
    (not read).
11. If `SkyTubeDisablePhase` evaluates above 0 and equals `SafeZonePhase`:
    `0x1419b0f00(GameState)`.
12. Broadcast `OnSafeZoneUpdated` when bound.
13. Build two small structs from (`LastCenter`, `LastRadius`, `P-1`) and
    (`NextCenter`, `NextRadius`, `P`) and pass them to `0x141781400` (not
    read; inferred AI or analytics notification).
14. For every entry of `AlivePlayers`: when the controller's PlayerState
    passes several checks (not fully read; bytes `+0x12ec` and `+0xd30` on
    the PlayerState), call `0x141a5b110(PlayerState, …)` and
    `0x1419ebd40(controller)`. `0x141a5b110` (read) writes two vectors, a
    float and a set flag into the PlayerState's `RespawnData` (`+0xd30`
    flag, `+0xd34`, `+0xd40`, `+0xd4c`), so this step refreshes each
    alive player's respawn location for the new circle; `0x1419ebd40`
    then sends the result data RPC (R6 section 5). Only respawn modes
    need it.

## 7. Stop at the playlist's last circle, `0x1419b0f50` (read)

`(GameState)`. Once only (`bPlaylistStoppedSafeZonePhases`):

- set the flag;
- if `0x1419996f0()` returns a time above 0: set `bIsInCountdown`, and
  `bIsInFinalCountdown` when that time is `<= TotalFinalCountdownTime`, then
  `0x1419b1000(GameState)`;
- call every mutator through slot `+0x8c0`.

## 8. Small helpers (read)

- `0x1419635f0(this)`: authority only; for every actor of one class in the
  world (class from `0x1446f1730`, not identified) call `0x1417fa5a0`.
- `0x1419634f0(this)`: authority only; for every Athena controller call
  `0x1424a51b0(controller[+0x1a40 object], 2)` (not named; inferred quest or
  stat tick for "storm phase changed").

## 9. What the client must see, and what one probe run should print

The client draws the storm from the replicated indicator only. Per second,
print: `GamePhase`, `bSafeZoneActive`, `SafeZonesStartTime`, `SafeZonePhase`,
and from the indicator `LastCenter`, `LastRadius`, `NextCenter`,
`NextRadius`, `SafeZoneStartShrinkTime`, `SafeZoneFinishShrinkTime`,
`bSafezoneEventDriven`, `bPaused`, plus `GameState.SafeZoneIndicator` being
non-null on the client.

Port order: the table helper (section 1.1), `IsSafeZoneAllowed`,
`StartSafeZonesPhase`, activation, the per-frame update, advance.

## 10. Open items

| Item | Why it matters |
|---|---|
| **Who fills `SafeZoneLocations`: found, bodies not read.** The game mode function `OnPlaylistDataLoaded` (contains `0x141955610`, see R1 section 6.1) calls `0x1418e7000(MapInfo, &SafeZoneLocations, …)` once and then sets `bSafeZoneLocationsInitialized` (game mode `+0x1000`). That wrapper calls `0x1418e7080` (1,915 lines), which calls `PickSafeZoneLocation` `0x1418f4530` (305 lines) per phase. `PickSafeZoneLocation` draws a random point in a disc (up to 10,000 tries), rejects points outside the allowed radius or inside reject volumes, and uses `ForceDistanceMin/Max`, `RejectRadius` and `RejectOuterDistance`; it logs `PickSafeZoneLocation[%d]: Setting Location`. A second writer, `0x14194dce0`, re-centers all circles on a given point and is called only from a mutator. | Without the `OnPlaylistDataLoaded` step the circle centers are empty. The stand-in server must run it before the storm activates. Read `0x1418e7080` before porting; it is the largest unread function on the G7 path. |
| **Storm damage: done.** The per-tick inside check is `0x141961aa0` and the per-pawn effect is `0x1419622e0`; both are in card R6 (sections 3 and 5), with the `GE_OutsideSafeZoneDamage` asset. | G8 "storm hurts" |
| `0x1419964d0` real start | section 3 step 5 |
| `0x141a5b1f0`, `0x1419ac150`, `0x141781400`, `0x1419ebd40` | **done**, R6 section 5 |
| `0x141a48240`, `0x141a5b110` | **done**, section 6 above |
| `0x141964450` full read | next-next circle |
