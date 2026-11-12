# R6: what starts the playlist setup, where the storm table lives, storm damage

Date: 2026-11-12. Read from the client image and the extracted content.
Confidence is "read" unless a line says "inferred" or "not read".
This card closes handoff items 1 to 3 and takes over the helper table that
was parked in `R-HANDOFF.md` section 3.

## 0. Corrections to earlier cards

| Earlier statement | Correct |
|---|---|
| `OnPlaylistDataLoaded` is at game mode vtable slot `+0xfc8` (handoff 3.2) | The slot is **`+0xd10`**. The table at `0x145dfb1b8` is another class's vtable (0x2b8 bytes long) followed directly by the game mode vtable, which starts at **`0x145dfb470`**. `vt.py` walks back over both. A second vtable that starts at `0x146155188` holds the same function at `+0xd10` (a subclass). |
| The call of slot `+0xfc8` at `0x1418f67b0` triggers it (handoff 3.2, item 1) | That call is in `0x1418f6760`, a virtual of `AFortAthenaSKVehicle` (vtable `0x145dd54d8`, slot `+0x2890`). It has nothing to do with the playlist. |
| `0x141978a80` "runs every storm update" and may hold storm damage | It spawns the per-phase building gameplay actors (supply drops). Section 4. |
| `SafeZoneDefinition` is twelve floats, "the last three are not named" (R2 1.1) | The last three are `PlayerCapSolo`, `PlayerCapDuo`, `PlayerCapSquad`. Section 2. |

`vt.py` caveat for later reads: when a hit reports a slot, check that the
reported vtable start has the usual head (`X`, `0x1406801c0`,
`0x142ec5d10`, `0x1406801e0`) and that no second head sits between the
start and the hit.

## 1. What triggers `OnPlaylistDataLoaded`

### 1.1 The chain (all four bodies read)

```
AFortGameStateAthena::OnRep_CurrentPlaylistInfo        0x1419a6040
  -> tail jump: LoadCurrentPlaylistData                0x14199f770
       async load, completion delegate:
       -> OnPlaylistDataLoadCompleted                  0x1419a3bb0
            (when no playspace plugin loads are pending)
            -> OnPlaylistFullLoadCompleted             0x1419a3fc0
                 -> Cast<AFortGameModeAthena>(World->AuthorityGameMode)
                      ->vtable[+0xd10]()   = OnPlaylistDataLoaded 0x141955570
                 -> 0x1419b47b0(GameState)   playlist values into the GameState
                 -> 0x1419a0870(GameState)   loot item definitions to load
```

Names come from the log strings inside each function
(`…in AFortGameStateAthena::LoadCurrentPlaylistData(), PlaylistName is %s
(Server Side)`, `Failed to find playlist data in OnPlaylistFullLoadCompleted`
and so on).

The playlist pointer every step uses is
`GameState.CurrentPlaylistInfo` (`+0x1cd0`): `OverridePlaylist` at `+0x1df8`
when set, else `BasePlaylist` at `+0x1df0`.

### 1.2 The steps

**`OnRep_CurrentPlaylistInfo`, `0x1419a6040`** (146 lines). Logs the
playlist name, then for every local player with a view target sets a string
built from the playlist's `+0x3c` field (rich presence or loading-screen
text; client-only cosmetics, not read further), then jumps to
`LoadCurrentPlaylistData`. Its only direct caller is its own exec thunk
(`0x1429202b0` region). **No server function in this image calls it.** The
server code that writes `CurrentPlaylistInfo` is not in the client;
`SetCurrentPlaylistId` (`0x141963c70`) only writes `CurrentPlaylistId` and
`CurrentPlaylistName` on the game mode and logs that it is deprecated.

**`LoadCurrentPlaylistData`, `0x14199f770`** (723 lines; first 180 read,
rest skimmed). Sets `bPlaylistDataIsActivelyLoading` (`+0xff1`), builds a
delegate to `OnPlaylistDataLoadCompleted`, then has two branches on `Role`:
authority logs "(Server Side)" and others log "(Client Side)". Both ask the
asset manager (slot `+0x330`) to load the playlist's bundle and bind the
delegate. So the function is written to run on the server as well.

**`OnPlaylistDataLoadCompleted`, `0x1419a3bb0`** (145 lines).

1. No playlist: log `Failed to find playlist data in
   OnPlaylistDataLoadComplete` and return.
2. On authority, when `PlayspaceManagerComponentCached` (`+0x298`) is set:
   copy the playlist's plugin list (playlist `+0xb8`) into the array at
   GameState `+0xf70`; unless the playlist passes `0x141a4a6f0`, also add
   `BattleRoyalePlayspacePlugins` (`+0xf60`) without duplicates.
3. For each plugin name, ask `0x14115f680`; each one that needs loading
   gets an async request with completion `0x1419a42b0`. The int at `+0xf50`
   counts pending loads.
4. When `+0xf50 < 1` and `+0xf54 < 1`: call `OnPlaylistFullLoadCompleted`
   now. Otherwise `0x1419a42b0` and `0x1419a4830` (plugin load and mount
   callbacks, not read past their last call) call it when the counters
   reach zero.

**`OnPlaylistFullLoadCompleted`, `0x1419a3fc0`** (80 lines). Runs once
(guard byte `+0xf58`).

1. No playlist: log and return.
2. `World = GetWorld()` (slot `+0x150`). When non-null:
   - `GM = 0x141641300(World)`: `World[+0x128]` (`AuthorityGameMode`) cast
     to `AFortGameModeAthena` (class getter `0x1428ee970`). When non-null,
     **`GM->vtable[+0xd10]()`**. On a client the game mode is null and this
     is skipped.
   - a per-world subsystem (`0x14463f750`) gets its `+0x418` set from
     playlist `+0x5a4`;
   - an owner-side object check then `0x144229900(World, playlist[+0x5c0])`
     (not read; guarded by a flag that is clear on a dedicated server,
     inferred).
3. `0x1419b47b0(GameState)` (639 lines, first 45 read): copies playlist
   values into the GameState, starting with `CachedSafeZoneStartUp`
   (`+0x1ef4`) from playlist `SafeZoneStartUp` (`+0x8c0`) and sizing
   `TeamXPlayersLeft` for large-team playlists.
4. `0x1419a0870(GameState)` (1,062 lines, first 45 read): logs
   `Determining Loot Based Item Defs to Load` and collects loot tables
   through playlist slots `+0x260` / `+0x268`.

### 1.3 What this means for the stand-in server

- The real trigger is: write `GameState.CurrentPlaylistInfo`
  (`BasePlaylist`, bump the replication key, mark the array dirty) and call
  `OnRep_CurrentPlaylistInfo` on the server. Everything else follows.
- In the reconstruction `AFortGameStateAthena::OnRep_CurrentPlaylistInfo`
  is a reflection stub ("Body not recovered"), and the three functions
  below it are not ported. Two ways to get the game mode step:
  1. **Port the chain** (`0x1419a6040`, the server branch of `0x14199f770`,
     `0x1419a3bb0`, `0x1419a3fc0`). This also gives the playlist-to-GameState
     copy (`0x1419b47b0`), which R2 needs for `CachedSafeZoneStartUp`.
  2. **Call the game mode directly**: after the MapInfo exists and the
     playlist object is loaded, call `OnPlaylistDataLoaded` (or its two
     inner calls: flight path R5 section 1, storm centers R5 section 2) and
     set `CachedSafeZoneStartUp` by hand. Smaller, and enough for G7.
- Either way the call must come **after** `GameState.MapInfo` is set and
  **before** `StartWarmupPhase` (R1 section 6.1). `OnPlaylistDataLoaded`
  itself has no retry: it runs once per `OnPlaylistFullLoadCompleted`.
- The client does the same chain from replication and skips the game mode
  call, so nothing extra has to be sent for this step.

## 2. Where the storm phase table comes from

### 2.1 Answer

`SafeZoneDefinition` is **instance data on the MapInfo actor placed in the
map**, not class defaults and not the playlist. In
`Athena/Apollo/Maps/Apollo_Terrain.umap` the actor is `DefaultMapInfo_2`
(class `DefaultMapInfo_C`) and carries exactly these overrides:

| Property | Value |
|---|---|
| `SafeZoneDefinition` | twelve scalable floats, each `Value=1` with curve table `AthenaGameData` and the row below |
| `SafeZoneVolumeDefinitions` | four entries, volumes `SafeZoneVolume_0`, `_4`, `_7`, `_9` (plain `TriggerVolume` brushes in the same level), rejection chance rows `Default.SafeZone.Volume.00` to `.03.RejectionChance` |
| `AircraftDropVolume` | `SafeZoneVolume_0` |
| `SafeZoneStorm` | `BP_SafeZoneStorm_2` (the visual storm actor, same level) |
| `CachedPlayableBoundsForClients` | origin (-256, 2048, 0), extent (128256, 126464, 19000), sphere radius 168209.23 |

The only other map with these rows is `Athena/Maps/Crucible/Athena_Crucible`.
No native code names the rows (no `Default.SafeZone.Radius` string in the
image), and `DefaultMapInfo`'s class defaults only reference
`Default.SafeZone.StartDelay`, `StartingRadius` and the aircraft rows.

`SafeZoneDefinition` member order (0x20 bytes each from MapInfo `+0x548`)
and rows, with the `AthenaGameData` values (`level:value`, linear between
keys):

| Member | Row `Default.SafeZone.` | Keys |
|---|---|---|
| `Count` | `Count` | 0:10 |
| `Radius` | `Radius` | 0:150000 1:80000 2:40000 3:20000 4:10000 5:5000 6:2500 7:1650 8:1090 9:0 |
| `ForceDistanceMin` | `ForceDistanceMin` | 0:0 |
| `ForceDistanceMax` | `ForceDistanceMax` | 0:0 |
| `RejectRadius` | `RejectRadius` | 0:0 |
| `RejectOuterDistance` | `RejectOuterDistance` | 0:0 |
| `WaitTime` | `WaitTime` | 0:0 1:200 2:120 3:90 4:80 5:50 6:30 7:0 |
| `ShrinkTime` | `ShrinkTime` | 0:0 1:240 2:120 3:90 4:70 5:60 6:60 7:55 8:45 9:75 10:0 |
| `MegaStormGridCellThickness` | `MegaStormGridThickness` | 0:0 |
| `PlayerCapSolo` | `PlayerCap.Solo` | 0:100 1:100 2:70 3:70 4:50 5:50 6:30 9:30 10:0 |
| `PlayerCapDuo` | `PlayerCap.Duo` | same as Solo |
| `PlayerCapSquad` | `PlayerCap.Squad` | same as Solo |

Related rows read by other code: `Default.SafeZone.StartDelay` 0:60,
`StartingRadius` 0:185000, `Damage` 0:0.01 2:0.01 3:0.02 4:0.05 5:0.08
6:0.1 9:0.1 10:0, volume rejection chances 0.9 / 0.7 / 0.5 / 0. Playlists
replace rows through their composite tables
(`Athena/Playlists/*/AthenaCompositeGD_*`), which is how a mode changes
the storm without touching the map.

### 2.2 Is spawning `DefaultMapInfo_C` enough for G7

**Not by itself.** A freshly spawned `DefaultMapInfo_C` has the aircraft
class, both aircraft boxes, speed, height and the storm start delay and
radius, but an empty `SafeZoneDefinition` (`Count` evaluates to its plain
value with no curve, so the phase count is wrong and every radius is the
same), no rejection volumes and no drop volume.

It becomes enough when the stand-in fills the instance data after the
spawn, before `OnPlaylistDataLoaded`:

1. Set the twelve `SafeZoneDefinition` scalable floats to `Value=1`, curve
   table `AthenaGameData` (the loaded object; a playlist's composite table
   when one applies), rows from the table above. This is plain property
   data and needs nothing from the map.
2. Leave `SafeZoneVolumeDefinitions` empty. The storm-center picker then
   skips the volume rejection test (R5 section 2 step with
   `SafeZoneVolumeDefinitions`), so centers are drawn only by radius from
   the previous center. Centers will differ from retail in distribution
   (no bias away from the map edge), which G7 does not test.
3. Leave `AircraftDropVolume` null. The flight path code then uses the
   `AircraftDropZone` box from the class defaults (R5 section 1 step 5).
4. `SafeZoneStorm` null: only the visual actor; the client has its own
   copy when it loads the real map. Not checked what the server does with
   it (no reads of that property found in the functions read so far).

**The validity check passes for a spawned instance.** `0x1418f1550` is
`IsMapInfoValid` (read, 106 lines; log strings `IsMapInfoValid: …`). It
tests properties only, nothing about how the actor came to exist:

1. `AircraftClass` (`+0x3b0`) is set and is a child of the aircraft base
   class (`0x1427cdfa0`);
2. `AircraftSpeed` (`+0x4b8`) at level 0 is above 0;
3. `AircraftSpawnZone` has `Min < Max` on both axes;
4. when `AircraftDropVolume` is set: valid, stop here;
5. otherwise `AircraftDropZone` has `Min < Max` on both axes and lies
   strictly inside `AircraftSpawnZone`.

The class defaults satisfy all of it (`AthenaAircraft_C`, speed row 7500,
spawn zone ±150000, drop zone ±80000). The one dependency is that
`AthenaGameData` is loaded, because the speed comes from its row
`Default.Aircraft.Speed`; with the table missing the scalable float
returns its plain value (1 in the asset, still above 0, but then the bus
moves at 1 unit per second).

Alternative that needs no hand-filled data: `Apollo_Terrain.umap`'s
persistent level is small (1.8 MB, 143 exports). It holds the MapInfo, the
four safe zone volumes with their brush models, the storm actor, world
settings, `FortPoiManager`, and 87 `LevelStreamingAlwaysLoaded` entries.
Loading the persistent level with the streaming entries suppressed would
give the real MapInfo and volumes without the terrain. Not tested; the
client would still load the full map, so replicated actor paths must match
(`DefaultMapInfo_2` is a level actor and would resolve by path on both
sides, which a run-time spawned MapInfo would not: a spawned one
replicates as a dynamic actor instead. Both work for `GameState.MapInfo`
replication; the difference matters only if the client expects the level
copy to be the one referenced. Not verified).

## 3. Storm damage per pawn, `0x1419622e0` (read, 168 lines)

Signature (from the caller `0x141961aa0`, card section 5 table):
`(GameMode, GameState, Pawn, int Phase, bool bInside, bool bFlag)`.
Returns at once for a null pawn.

Names: game mode `+0x14f0` storm camping timer handle (unnamed in
reflection), `+0x14f8` float, the current camping multiplier (unnamed),
`+0x1500` `StormCampingInitialDelayTime`, `+0x1520`
`StormCampingIncrementTimeAfterDelay`, `+0x1540`
`StormCampingDamageMultiplier`. Pawn `+0x1220` bit 0 `bIsInAnyStorm`,
bit 1 `bIsInsideSafeZone`, `+0x1224` an active gameplay effect handle
(8 bytes, `-1` when none; the handle of `GE_OutsideSafeZoneDamage` applied
by `FinishRestartPlayer`, D2 section 10; inferred), `+0x53a` bit 1
`bIsDBNO`. GameState `+0x20c0` `SignalInStormRegenSpeed`, `+0x20c8` to
`+0x20d4` `StormCNDamageVulnerabilityLevel0` to `Level3`.

Tags, from the native tag table (`0x141ed1800()`, registered in
`0x141ebb310`): `+0xfa0` `SetByCaller.SignalInStorm`, `+0xfa8`
`SetByCaller.StormDamageBuffCNTag`, `+0xfb0`
`SetByCaller.StormCNKillDamage`, `+0xfb8` `SetByCaller.StormCampingDamage`.

Steps:

1. **Camping timer (one timer for the whole game mode, not per pawn).**
   - `bInside` false: when the timer at `+0x14f0` does not exist or is
     paused, and `StormCampingIncrementTimeAfterDelay(Phase) > 0`: set a
     looping timer, rate = that value, first delay =
     `StormCampingInitialDelayTime(Phase)`, callback `0x14194cd50`.
     The callback does `+0x14f8 += StormCampingDamageMultiplier(Phase)`.
   - `bInside` true: when the timer exists and is not paused, clear it,
     zero the handle and set `+0x14f8 = 1.0`.
   - As written, any inside pawn resets what any outside pawn started,
     because the function runs for each pawn in turn. With the default
     rows (`StormCamping*` all 0) the timer never starts, so the port can
     keep the literal behavior at no cost.
2. `bHasTag = 0x14232d430(Pawn)`: the pawn's ability system component has
   the tag `Granted.Athena.Item.DogSweater.InDamageZone` (table `+0x2e0`).
   `0x14234d960(Pawn, bHasTag ? bInside : !bInside)`: on authority, when
   the value differs, write `bIsInAnyStorm` and call `0x14233c4e0`
   (section 3.1).
3. Pawn slot `+0x1010(bInside)` = `0x14234d990` (section 3.1). The same
   slot is what the Warmup branch of the caller uses. **This is the switch
   for the damage:** it adds or removes the loose tag
   `Gameplay.InsideSafeZone`, which the effect ignores on.
4. `ASC = Pawn's IAbilitySystemInterface (+0x4c8) ->
   GetAbilitySystemComponent()`. Stop when null or when the handle at
   `+0x1224` is `-1`.
5. `ASC->slot[+0x5f0](Handle, Phase)`: set the active effect's level to the
   phase. Then find the active effect (`0x1415f7fe0`); stop when not
   found. All magnitudes below are set-by-caller writes on its spec
   (`0x1407303d0(Spec, Tag, float)`).
6. `StormCampingDamage = max(1.0, GameMode[+0x14f8])`.
7. Stop here when `bFlag` is false. Otherwise `StormCNKillDamage = 0`, then:
   - **Pawn alive** (`0x142331080(Pawn)` false: health above 0 and not
     being destroyed) **and not DBNO**:
     - outside: level = `Phase + 1`; `SignalInStorm = 0`.
     - inside: level = `0`; `SignalInStorm = SignalInStormRegenSpeed`.
     - `P = Pawn->slot[+0xb48]()` = `0x142323de0`: the current value of
       the `SignalInStorm` attribute (`HealthSet + 0xd0`, value at `+0xdc`;
       0 when the pawn has no health set).
       - `P <= 0`: `StormCNKillDamage = 1000.0`.
       - `0 < P < 100`: `i = clamp((100 - (int)P) / 25, 0, 3)`;
         `StormDamageBuffCNTag = StormCNDamageVulnerabilityLevel[i]`.
       - `P >= 100`: `StormDamageBuffCNTag = 0`.
   - **Dead or DBNO**: level = `0`; `StormDamageBuffCNTag = 0`;
     `SignalInStorm = 0`.

`GE_OutsideSafeZoneDamage` (asset read): infinite duration, period 1 s,
executes `FortDamageFormulaExecutionCalculation` with damage from row
`Default.SafeZone.Damage` at the effect level, one set-by-caller modifier
on `SetByCaller.StormCampingDamage`, cue
`GameplayCue.Athena.OutsideSafeZoneDamage`, and **ongoing requirement:
ignore tags `Gameplay.InsideSafeZone`, `Goose.PlayerStormImmune`,
`Granted.Athena.Item.DogSweater.InDamageZone`**. So the effect is always on
the pawn and is switched by the `Gameplay.InsideSafeZone` tag (set through
step 3, inferred) and scaled by its level.

**For G8 "storm hurts" the minimum is:** apply the effect once (already
done by `FinishRestartPlayer`), keep its handle in the pawn
(`SafeZoneAppliedGE` is the property at `+0x1230`; the handle read here is
at `+0x1224`), and each storm tick per pawn do step 3 (the tag), step 5
(level = phase) and step 6 (`StormCampingDamage = 1.0`). Step 7 does not
run in an ordinary match. The `StormCampingDamage` write is required: the
effect multiplies by it (inferred from the modifier; an unset set-by-caller
logs an error and evaluates as 0).

`bFlag` is `0x14199e4e0(GameState)`, evaluated once per tick in
`0x141961aa0`. That is the mode test in section 5: false unless a global
option is set and the playlist qualifies. Together with the names
(`StormCN…`, `WinningTeamsCN`, the `SignalInStorm` attribute) this is the
regional "signal in storm" rule set, and **in an ordinary match `bFlag` is
false, so the function ends after step 6.** The effect level then stays
at `Phase` for every pawn, inside or outside, and only the
`Gameplay.InsideSafeZone` tag decides whether damage is dealt.

The phase passed in is `SafeZonePhase`, except that a pawn inside one of
the per-volume storm overrides gets that volume's phase (`+0x45c` of the
volume; `0x141961aa0` lines 160 to 222, skimmed).

### 3.1 The pawn side (read)

**`0x14234d990`, pawn slot `+0x1010`, `(Pawn, bool bInside)`.** Authority
only, and only when `bIsInsideSafeZone` (`+0x1220` bit 1) differs:

1. write the bit;
2. on the ability system component (interface slot `+0x10`): read the
   current loose count of `Gameplay.InsideSafeZone` (tag table `+0x928`,
   count map at ASC `+0x10e0`) and apply the difference so the count
   becomes 1 when inside and 0 when outside (`0x140744d30(ASC + 0x1040,
   Tag, Delta, 0)`);
3. broadcast `OnSafeZoneOccupancyChangedEvent` (`+0x21e8`) with the new
   value.

**`0x14233c4e0`, after `bIsInAnyStorm` changes.** Works out whether the
storm screen effects should play (a world check `0x144693a30` and the
local view `0x142321a70`), updates `bPlayingSafeZoneEffects` (`+0x1061`
bit 1), calls `0x142a77180` and broadcasts the delegate at `+0x1238`, then
pawn slot `+0x1018`. Cosmetic; the server can skip everything but the bit.

**`0x14231c4c0`, pawn slot `+0xb28`:** health. 0 when the object is being
destroyed, 1.0 when the pawn does not use stats or has no health set,
else `max(0, Health)`.

## 4. `0x141978a80`: per-phase building gameplay actor spawns (read, 333 lines)

Called from `0x141975090` only. Not storm damage.

Runs only when the GameState is `AFortGameStateAthena`, `GamePhase`
(`+0x1cb0`) is 3 or 4, `MapInfo` (`+0x1c98`) and the world exist.

For each entry of game mode `BuildingGameplayActorSpawnData` (`+0x1300`,
20 bytes: `[0]` index into MapInfo `BuildingGameplayActorSpawnDetails`
(`+0x2b8`, 0x150 bytes each), `[1]` spawned so far, `[2]` number to spawn,
`[3]` next spawn time as float or `-1`, `[4]` a flag byte):

1. When the next spawn time is set and reached: clear it. With a valid
   detail entry whose class (`detail[0]`) is a building gameplay actor:
   count the live actors of that class (`0x141ef5370`, plus those of the
   second class at `detail[1]`), compare with the cap scalable float at
   `detail + 0x30`; under the cap, pick a location with `0x1418f4190`
   (MapInfo, current `SafeZoneLocations[Phase]` and `Radius[Phase]` from
   the cached table; the supply drop picker, log string
   `PickSupplyDropLocation: …`), take a height from the scalable float at
   `detail + 0x10`, a random yaw, spawn (`0x141bc5dc0`), finish spawning
   and call the actor's slot `+0x5e8`. Count it in `[1]`.
2. Schedule the next one: while `[1] < [2]` the next time is `now +
   random(min, max)` from two more scalable floats of the detail; when the
   phase changes, `[2]` is redrawn as `random(min count, max count)` at the
   phase.

This is the supply drop (and llama-like) scheduler. It is G9+ content and
depends on `BuildingGameplayActorSpawnDetails`, which `DefaultMapInfo`'s
class defaults do carry (`SupplyDropInfoList`, rows
`Default.SafeZone.SupplyDropMinCount` and so on). Skip for G7 and G8.

## 5. Helpers read after the R5 commit

Moved here from the handoff. Confidence "read" unless stated.

| Address | Result |
|---|---|
| `0x1418f8580` | Fills the flight info struct. Speed is the MapInfo scalable float at `+0x4b8` (`AircraftSpeed`). Writes start location (`+0x0`), height (`+0x8`), yaw in degrees (`+0x10`), speed (`+0x18`), time to flight end (`+0x1c`, distance / speed), time to drop start (`+0x20`), time to drop end (`+0x24`, never below drop start). When global `0x1478c2420` is set, the start point is moved back so the drop start comes after the scalable float at MapInfo `+0x4d8`. |
| `0x1419f5840` | Per-controller bus placement helper. Runs only when controller slot `+0x6a8` returns true. Calls `0x1423a1e80`, `0x142392ff0`, `0x142393980(PC,0,1,1,0)`, clears byte `+0x15a` in a per-world record, then `0x1418b99f0(Aircraft, 0x1419918d0(GameState, PC), 0)`. The inner three calls are not read. |
| `0x1419ac150` | `GameState.SafeZoneIndicator` (`+0x1770`) null check, then `0x141a5b1f0`. |
| `0x141a5b1f0` | On the indicator: stores radius at `+0x39c`, then sets the transform of the component at `+0x3a0` to location = center, scale = (radius × float `+0x398`, same, 1000). This is the visible storm wall. |
| `0x141963e70` | Pause or resume the storm. `0x1419af240(GameState, bPause)`. On pause: `TimeRemainingWhenPhasePaused = indicator[+0x254] − now`. On resume: `indicator[+0x254] = now + TimeRemainingWhenPhasePaused`. |
| `0x141946150` | Post-match delay. `EndGameKickPlayersDelay_NoIGMM` when the runtime option check `0x1426f5b80` fails, else `EndGameKickPlayersDelay_IGMM + 0x1419920d0(GameState)`. |
| `0x1428ef5b0` | Not the engine `EndMatch`. It calls a Blueprint event by name through slot `+0x210` (`ProcessEvent`). The name global is `0x1481962d0`; not resolved. |
| `0x14199e4e0` | Playlist mode test: false unless a global option (`0x141ed2350` `+0x2c8`) is set; false when the playlist passes `0x141a49590` or `0x141a4b0d0`; false when the playlist has a specific gameplay tag or its byte `+0x467` is not 0 or 7. |
| `0x14194de20` | Sets `bSafeZoneLocationsInitialized`. When no controller reports state 2 through slot `+0x730`, it first calls the storm-center setup `0x1418e7000(GameState[+0x1c98], &SafeZoneLocations, this+0xda8)`. |
| `0x1419783c0` | Placement update for some playlist types: refreshes every PlayerState (`0x141a5adb0`, `0x141a51eb0`), calls `0x141938090`, then for the losing team's members sets place (`0x141a5b600`) and state 2 (`0x141a5b610`), and for the winner's team state 1. |
| `0x1419ebd40` | Controller: collects match result data from the PlayerState (`0x141a481a0`) and sends a client RPC through thunk `0x142a53030`. RPC name not resolved. |
| `0x141961aa0` | Storm inside check, 399 lines. Skips when game mode byte `+0xda0` is set. In Warmup it calls pawn slot `+0x1010(bInside)` for every player pawn. Otherwise, for each pawn: `bInside = 0x14194e9e0(GameMode, &PawnLocation, 0x1419494f0(GameMode))`, an optional creative-volume override, then `0x1419622e0` (section 3). |
| `0x14196d120` | `StartEndGamePhaseTeam`. Read; card R3 section 5.1. |
| `0x141962af0` | Logs `MatchStarted,%s,%s` / `MatchEnded,%s,%s`. Body not read, 131 lines. |
| `0x141781400` | Analytics event `Athena.NewSafeZone` (`LastLocation`, `LastRadius`, `NewLocation`, `NewPhase`). Not gameplay. |
| `0x1419fc560` | Contains `[LTMLoading] - Player %s StartWaitLTMLoading, IsServer %s`. Body not read. |
| `0x1418f1550` | `IsMapInfoValid`, section 2.2. |
| `0x141641300` | `World[+0x128]` (`AuthorityGameMode`) cast to `AFortGameModeAthena`, else null. |
| `0x14194cd50` | Storm camping timer callback (section 3 step 1). |
| `0x14232d430` | Pawn: its ability system component (`+0xbb8`) has native tag `+0x2e0`. |
| `0x14234d960` | Pawn, authority only: set `bIsInAnyStorm` when changed, then `0x14233c4e0`. |
| `0x142331080` | Pawn "is dead": true unless health (slot `+0xb28`) is above 0 and the object is not flagged for destruction. |

## 6. Still not read

| Address | What |
|---|---|
| `0x1419a42b0`, `0x1419a4830` | playspace plugin load callbacks |
| rest of `0x1419b47b0`, `0x1419a0870` | playlist-to-GameState copy, loot definition preload |
| `0x141962af0`, `0x1419fc560` | match start and end log line, LTM loading wait |
| per-volume phase override in `0x141961aa0` (lines 160 to 222) | only for modes with extra storm volumes |
| `0x141988940`, `0x1419a9580`, `0x1419ae490`, `0x1419797b0` | captured earlier, not read |
