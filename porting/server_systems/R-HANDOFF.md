# R cards: handoff for the next Fable 5.2 session

Date: 2026-11-12. Written at the user's request when the previous session
wound down. Read this before touching the R cards.

## 1. Division of work (set by the user)

- **Sonnet implements** G6 to G9 in `porting/native/` from cards R1 to R5.
  The Fable session does not implement there.
- **Fable reads and documents**: the remaining unread client functions, and
  the hard questions in section 4. It also reviews Sonnet's re-creation
  designs (death sequence, building, inventory and pickup, bus jump) when
  they exist.
- Workspace rules are in `/opt/AGENTS.md` and the memory index. The ones that
  bit this session: commit with
  `git commit --only --author="Claude Fable 5.2 <noreply@anthropic.com>" -- <paths>`
  plus `GIT_COMMITTER_NAME/EMAIL`, because a plain `git commit` takes other
  sessions' staged files. Push fast-forward only. Never run a directory-wide
  `git checkout`.

## 2. What exists

| File | Content |
|---|---|
| `R1-warmup-and-aircraft-code.md` | tick driver, warmup, ready counter, aircraft phase, bus delegates |
| `R2-safe-zone-code.md` | storm start, update, advance, cached definition table |
| `R3-elimination-and-match-end-code.md` | alive-player lists, win check, end game, match end |
| `R4-readiness-build-interact-code.md` | `bReadyToStartMatch`, the build RPC correction, interact |
| `R5-playlist-setup-phase-handler-receipts.md` | flight path and storm-center setup, phase handler, full match order (section 6), unread list (section 7) |

`G6_MAPPING_APPROACH.md` and section 0 of `SERVER_FIRST_PLAN.md` link to
them. Last pushed doc commit of the previous session: `e89b15e1`.

Commit `f8742834` wrongly included a Codex session's 33 staged test renames
and a Codex author. The user was told and has not asked for a revert. Leave
it unless asked.

## 3. Findings not yet in the cards

These were read after the last card commit. Merge them into the cards' open
item tables (R1 section 11, R2, R3, R5 section 7) and mark the items done.

### 3.1 Helpers read (confidence: read unless stated)

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
| `0x141961aa0` | Storm inside check, 399 lines. Skips when game mode byte `+0xda0` is set. In Warmup it calls pawn slot `+0x1010(bInside)` for every player pawn. Otherwise, for each pawn: `bInside = 0x14194e9e0(GameMode, &PawnLocation, 0x1419494f0(GameMode))`, an optional creative-volume override, then **`0x1419622e0(GameMode, GameState, Pawn, Phase, bInside, bFlag)`**. That last function is the per-pawn storm effect and is **not read**. It is the most useful next read for G8 storm damage. |
| `0x14196d120` | Is `StartEndGamePhaseTeam` (log string `[AFortGameModeAthena::StartEndGamePhaseTeam()] TeamNum [%d], WinningPS [%s], Placement [%d…`, and `FortGameModeAthena: Team %d placed %d!`). Body not read, 214 lines. |
| `0x141962af0` | Logs `MatchStarted,%s,%s` / `MatchEnded,%s,%s`. Body not read, 131 lines. |
| `0x141781400` | Analytics event `Athena.NewSafeZone` (`LastLocation`, `LastRadius`, `NewLocation`, `NewPhase`). Not gameplay. |
| `0x1419fc560` | Contains `[LTMLoading] - Player %s StartWaitLTMLoading, IsServer %s`. Body not read. |

The decompiler resolved wrong starts for `0x141a48240`, `0x141a5b110`,
`0x1419ae460`, `0x1419997b0` and `0x141a483a0`. Those need an exact capture
(`work/claude-plan/req_exact.json`) and are still unread. Captured but not
read: `0x141988940` (476 lines), `0x1419a9580` (336), `0x1419ae490` (126),
`0x141978a80` (382, runs every storm update), `0x1419797b0` (232),
`0x1426b0790` (184, the only caller of the extra-piece build helper).
The decompile cache was in a session scratch directory and may be gone;
re-capture with `tools/rd.py` (one at a time, it shares `req_rd.json`).

### 3.2 `OnPlaylistDataLoaded`

- Real start is `0x141955570` (not `0x141955610`, which is a mid-function
  chunk). It is a virtual at slot `+0xfc8` of the game mode vtable
  `0x145dfb1b8`.
- The only call of slot `+0xfc8` in the game code range is at
  `0x1418f67b0`, inside a function in the MapInfo code region. **Not read
  yet.** Reading it answers "what triggers the flight path and storm-center
  setup on the server", which decides whether Sonnet can call it directly
  after the MapInfo exists.

### 3.3 MapInfo without the full map

- `FortniteGame/Content/Athena/Balance/MapInfos/DefaultMapInfo` (under
  `/mnt/vibrato/fortnite-extracted-full`) is a Blueprint, class
  `DefaultMapInfo_C`. Its class defaults hold `AircraftClass =
  AthenaAircraft_C`, `AircraftSpawnZone` (FBox2D ±150000),
  `AircraftDropZone` (FBox2D ±80000), and scalable floats on
  `AthenaGameData` rows (`Default.Aircraft.DeviationAngle`,
  `DistanceFromMidLine`, `Height`, `Speed`, `Default.SafeZone.StartDelay`,
  `StartingRadius`).
- `SafeZoneDefinition` and `FlightInfos` do **not** appear in those
  defaults, so the per-phase storm table comes from somewhere else
  (candidates: the instance placed in the map, the playlist, or curve
  tables read by the cache builder in R2). Not settled.
- Idea to test (not verified): spawn `DefaultMapInfo_C` on the stand-in map
  and set `GameState.MapInfo`, instead of loading `Apollo_Terrain`. The
  game mode Blueprint's parent `AFortGameModeBR` is missing from the native
  class set (card C); that is a separate blocker for using the real game
  mode.
- `MapInfo` string references worth reading: `0x1418f1550`,
  `0x141943d87`, `0x141992a10`. Reflection shows
  `GameState.bMapInfoInitialized`, `OnRep_MapInfo`, `OnMapInfoInitialized`.

## 4. Open work for Fable, in priority order

1. Read the function containing `0x1418f67b0` and write up the
   `OnPlaylistDataLoaded` trigger.
2. Settle where the storm phase table comes from and whether spawning
   `DefaultMapInfo_C` is enough for G7. Write it as card R6.
3. Read `0x1419622e0` (per-pawn storm effect) and `0x141978a80`, add to R2.
4. Read `0x14196d120`, `0x1426b0790` and the wrong-start helpers; update R3
   and R4.
5. Re-run the RPC sweep with a "real but only a guard" rule (R4 section 5).
6. Review Sonnet's re-creation designs when they appear.

## 5. Tools

- `python3 -I porting/tools/rd.py 0xADDR Class --full` (annotated
  decompile), `callers.py`, `vt.py`, `strx.py`, `strs_near.py`,
  `asset_props.py`.
- `objdump -D -b binary -mi386:x86-64 -M intel --adjust-vma=0x140000000` on
  `porting/generated/dump/FortniteClient-Win64-Shipping.runtime.exe` for
  return values and discarded registers.
- Offsets to property names: `generated/reflection/bool_bits.json`
  (`by_class`). The two small scripts used last session (`off.py`,
  `view.py`) lived in the session scratch directory; they are a few lines
  each and easy to rewrite.
