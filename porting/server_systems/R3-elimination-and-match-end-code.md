# R3: removal from the alive list, win check and match end, read line by line

Date: 2026-11-12. Detailed read of the real server code that G8's last step
(elimination) and all of G9 run through. Same conventions and confidence
labels (**read**, **inferred**, **not read**) as
`R1-warmup-and-aircraft-code.md`. It refines `D2-match-flow.md` section 9
and `D6-combat-and-elimination.md` section 1. The death sequence that calls
into this code is compiled out and stays a re-creation (D6 section 2); this
card tells that re-creation exactly what it must pass in and what it gets
back.

## 0. Corrections to earlier cards

1. **`StartEndGamePhase` starts at `0x14196cb30`**, not `0x14196cb50`. The
   later address is a chunk in the middle of the function, and its decompile
   is unusable (registers instead of arguments). Use `0x14196cb30`.
2. **The win check runs only in the Aircraft and SafeZones phases** (section
   3 step 1). A removal during Warmup never ends the match. D2 and D6 do not
   say this. For a one-client test this means the match can only be ended by
   the win check after the bus has started.
3. `RemoveFromAlivePlayers` reads the old `PlayersLeft` and `TeamsLeft`
   **before** it changes them and passes both to the win check. The
   re-created death sequence must not update those two properties itself.

## 1. Names

`AFortGameModeAthena`: `+0xdd0` / `+0xdd8` `AlivePlayers` array / count
(controllers), `+0xdf0` `TeamAlivePlayers` (100 ints), `+0xdc0` timer handle
set at match end, `+0x1088` "match has been ended" byte (not reflected;
written only by the end-match helper, section 5).

`AFortGameStateAthena`: `+0x11f0` `bForceTeamScorePlacementOnDeath`,
`+0x12f0` `PlayersLeft`, `+0x1360` `TeamXPlayersLeft` (array), `+0x1370`
`WinningPlayerList` (array of player ids), `+0x1380` `TeamsLeft`, `+0x1388`
`WinningTeamsCN`, `+0x1408` `bIsLargeTeamGame`, `+0x1410`
`WinningPlayerState`, `+0x1418` (int, players at the moment the bus
unlocked, see R1 section 7.1), `+0x1428` `WinningTeam`.

`AFortPlayerControllerAthena`: `+0x220` `PlayerState`, `+0x248` `Pawn`,
`+0x590` team interface (slot `+0x28` returns the team byte), `+0x7f8`
`LastDamager`, `+0x808` `LastFallInstigator`, `+0x2fb8` `bMarkedAlive`.

`AFortPlayerStateAthena`: `+0x21c` `PlayerId`, `+0x318` team interface,
`+0x1288` `SecondsAlive`, `+0x1294` and `+0x1298` (two ints after
`bIsDisconnected`, not named; `+0x1298` is the time written when the bus
unlocks).

`UFortPlaylistAthena`: `+0x5c` `MaxTeamSize`, `+0xc8` `DefaultFirstTeam`,
`+0x467` `WinConditionType`, `+0x518` `WinConditionPlayersRemaining`.

`EDeathCause` values used here: Banhammer 18, RemovedFromGame 19,
LoggedOut 47, TeamSwitchSuicide 48, WonMatch 49. `EAthenaWinCondition`:
LastManStanding 0, TimedLastMenStanding 4.

## 2. `RemoveFromAlivePlayers`, `0x14195e620` (read)

`(this, AFortPlayerControllerAthena* PC, FinisherPlayerState, FinisherPawn,
FinishingWeapon, uint8 DeathCause, bool bSkipWinCheck)`.

Direct callers in the client: `Logout` (`0x141950f10`) and the team-change
handler (`0x14195a3d0`). The death path's call is compiled out.

1. Needs an Athena GameState and a playlist, else return.
2. `OldPlayersLeft = GameState.PlayersLeft`,
   `OldTeamsLeft = GameState.TeamsLeft`.
3. When `PC` is not null:
   - zero `PC.LastDamager` and `PC.LastFallInstigator`;
   - unless `DeathCause` is Banhammer, RemovedFromGame, LoggedOut,
     TeamSwitchSuicide or WonMatch, ask every mutator through slot `+0x838`
     with a bool that starts true. If a mutator clears it, log
     `RemoveFromAlivePlayers called for Player [%s] who wasn't in the
     AlivePlayers list.` and skip to step 4;
   - remove every entry equal to `PC` from `AlivePlayers`. If nothing was
     removed, log the same warning and skip to step 4;
   - `PC.bMarkedAlive = 0`;
   - `Team = PC team byte`; `Index = Team - Playlist.DefaultFirstTeam`. If
     `Index` is outside 0..99 log `doesn't have a valid PvP team`. Else
     `TeamAlivePlayers[Index] -= 1`, and when that reaches 0 and
     `GameState.TeamsLeft > 0`: `GameState.TeamsLeft -= 1`;
   - when the index is invalid or the team now has no one alive, remove the
     index from the set at game mode `+0x1468` (not named);
   - **`GameState.PlayersLeft = AlivePlayers.Num()`**;
   - when `bIsLargeTeamGame` and the index is valid:
     `0x141989240(GameState, Index)` and `TeamXPlayersLeft[Index] -= 1`;
   - log `Player [%s] removed from alive players list (Team [%d]). Player
     count is now [%d]. PlayerBots count is now [%d]. Team count is now
     [%d].`
4. When `bSkipWinCheck` is false:
   - if `DeathCause == LoggedOut` and `0x14199e430(GameState)` is true
     (a special mode test, not read): when no players are left and no one
     is spectating the leaver, log `CheckEndMatchOnLogout() No players left`
     and end the match with reason 4 (section 5);
   - call the win-check wrapper `0x141937f90(this, PC, FinisherPlayerState,
     OldPlayersLeft, OldTeamsLeft, FinisherPawn, FinishingWeapon,
     DeathCause)`. It lets every mutator handle the removal first (slot
     `+0x758`; a mutator that returns 2 stops it) and otherwise calls
     `CheckWinConditionLastManStanding`.

## 3. `CheckWinConditionLastManStanding`, `0x141937590` (read)

`(this, RemovedPC, FinisherPlayerState, int OldPlayersLeft, int
OldTeamsLeft, FinisherPawn, FinishingWeapon, uint8 DeathCause)`. Returns
bool (true when it ended the match).

1. Gate `0x14194e680(this)`. Returns false, and so the whole check does
   nothing, when: the match-ended byte `+0x1088` is set; or a mutator of one
   class (class from `0x14281c960`) is present and its `0x141b22920` test is
   false; or **`GamePhase` is not Aircraft or SafeZones**.
2. Needs an Athena GameState and a playlist. Return 0 when
   `GameState.bForceTeamScorePlacementOnDeath` is set.
3. Special modes first, each returning early: the `0x14199e430` mode (uses
   `TeamsLeft > 1` to keep going) and the `0x14199e4e0` mode (builds
   `WinningTeamsCN`). **Not read in detail; not used by the default solo
   playlist as far as the conditions show. Confirm both tests are false for
   `Playlist_DefaultSolo` before skipping them.**
4. Solo path. Taken when `Playlist.MaxTeamSize < 2` or
   `GameState.TeamsLeft > 1`:
   - if `MaxTeamSize == 1`, `OldPlayersLeft >= 2` and
     `GameState.PlayersLeft == 1`: the winner is `AlivePlayers[0]`. Log
     `Player [%s] is the last player alive. Starting end game.`; call
     `0x1419783c0(this, 0, FinisherPlayerState, RemovedPC)` (not read); call
     **`StartEndGamePhase(this, AlivePlayers[0], FinisherPawn,
     FinishingWeapon, DeathCause)`**; return 1;
   - otherwise, only for `WinConditionType == TimedLastMenStanding`: when
     `OldPlayersLeft > WinConditionPlayersRemaining` and `PlayersLeft` now
     equals it, call `0x1419783c0` and `0x14194b040(this)`. Return 0.
5. Team path (`MaxTeamSize >= 2` and `TeamsLeft <= 1`):
   - winning team = first index with `TeamAlivePlayers[i] > 0`, plus
     `DefaultFirstTeam`;
   - the finisher is kept only if they are on that team; otherwise the
     PlayerState of `AlivePlayers[0]` is used;
   - `Playlist_NowPlaying` is special-cased by name (ignored unless its last
     player died);
   - `0x1419783c0`, then the team win `0x14196d120(this, Team,
     PlayerState, 1, FinisherPawn, FinishingWeapon, DeathCause)` (214 lines,
     not read), then end the match with reason 0 and the team in a one-byte
     array. Return 1.

**Test consequence:** with one vanilla client, `OldPlayersLeft` is 1 when
that client dies, so step 4's condition `OldPlayersLeft >= 2` fails and
nobody wins. Victory can only be shown with two clients, or with one client
plus a second controller in `AlivePlayers` (a server-side dummy) that is
removed while the client is alive.

## 4. `StartEndGamePhase`, `0x14196cb30` (read)

`(this, WinnerPC, FinisherPawn, FinishingWeapon, uint8 DeathCause)`.

When `WinnerPC` and its PlayerState exist:

1. Log `%s won the match!` and the `StartEndGamePhase()` detail line.
2. `0x1419f4520(WinnerPC, 1)` (not read).
3. Winner PlayerState: `SecondsAlive = 0x141a483a0(PlayerState)`;
   the int at `+0x1294` = `GameState[+0x1418] - PlayerState[+0x1298]`.
4. `Team = WinnerPC team`. GameState slot `+0x878` with the team returns the
   team info actor; when it is the Athena team info class, write the int 1
   at team info `+0x380` (inferred: team place = 1).
5. `0x142a52de0(WinnerPC, FinisherPawn, FinishingWeapon, DeathCause)` and
   `0x142a3bda0(WinnerPC)` (not read; by the arguments the first is the
   server side of **`ClientNotifyWon`**).
6. GameState: `WinningTeam = Team`, `WinningPlayerState = winner's
   PlayerState`, append the winner's `PlayerId` to `WinningPlayerList` if it
   is not there, then slot `+0x5e8` (inferred: force net update).
7. If the winner has an Athena pawn: `0x141a2bff0(Pawn, 6, 1.0, 0)` (not
   read; inferred to make the pawn invulnerable or play the win state).

Then always: end the match with reason 0 and a one-byte array holding
`GameState.WinningTeam` (3 when there is no GameState). Returns 1.

The other direct caller, `0x141aa55f0`, is in a mutator.

## 5. The end-match helper, `0x14193df50` (read)

`(this, uint8 Reason, TArray<uint8>* WinningTeams, int Count)`; the
decompile shows only `this`. Eleven callers, all "the match is over" sites.

1. `0x1428ef5b0(this)` (not read; inferred to be the engine `EndMatch`,
   which leads to `HandleMatchHasEnded`).
2. `0x141963e70(this, 1)` (not read).
3. Walk `AlivePlayers`; for each Athena pawn call a function that is an
   empty stub in the client.
4. If `GameState.SpecialActorData` is set: `0x1417d1ad0(this)`.
5. Game mode slot `+0x828`.
6. Set the match-ended byte `+0x1088 = 1`. This is what stops further win
   checks (section 3 step 1).

## 6. `HandleMatchHasEnded`, `0x14194b750` (read)

1. `Delay = 0x141946150(this)` (not read; the post-match time).
   `0x1419ae460(GameState, …, Delay)` (not read; inferred to publish the end
   time to clients).
2. Set a timer of `Delay` seconds on handle `+0xdc0` (callback not read;
   D2 says it is the post-match shutdown).
3. Set the phase to EndGame through the phase setter (R1 section 3.1) and
   log `Set AthenaGamePhase to [EndGame].`
4. `0x141962af0(this, 0)` (not read).
5. Tell the game session helper, clear timers `+0xcf8` and `+0x14e0`, drop
   the delegate stored at `+0x1460`, and notify the game session
   (`0x142178c40`).

## 7. What the re-created death sequence must do with this

In D6 section 2 terms:

- Step 6 becomes exactly `RemoveFromAlivePlayers(PC, KillerState,
  KillerPawn, KillerWeaponDef, DeathCause, false)`. Do not touch
  `PlayersLeft`, `TeamsLeft`, `AlivePlayers` or `TeamAlivePlayers` anywhere
  else.
- The victim's place is `GameState.PlayersLeft + 1` read **after** that
  call.
- Step 9 (winner) is already done by `StartEndGamePhase`: winning team,
  winning player, the won notification. The re-creation only adds what that
  function does not do: the winner's `Place = 1` and the stats receipts.
- A player must be in `AlivePlayers` and `TeamAlivePlayers` to be removable.
  That is done by `AddToAlivePlayers` (section 7.1).

### 7.1 `AddToAlivePlayers`, `0x141933cb0` (read)

`(this, AFortPlayerControllerAthena* PC)`. Callers: `PostLogin`
(`0x141959680`), `FinishRestartPlayer` (`0x141943240`), `0x141959d80` and a
mutator.

1. Needs `PC`, an Athena GameState, and a playlist (or the special world
   test `0x1446939f0`).
2. Return if `PC` is already in `AlivePlayers`.
3. `Index = PC team - Playlist.DefaultFirstTeam` (first team 3 when there is
   no playlist). Outside 0..99: log `doesn't have a valid PvP team, and
   won't be added to the alive players list.` and return. **So the team
   must be assigned (`PickTeam`) before this runs.**
4. Append `PC`; `TeamAlivePlayers[Index] += 1`, and when that becomes 1,
   `GameState.TeamsLeft += 1`.
5. `PC.bMarkedAlive = 1`; `GameState.PlayersLeft = AlivePlayers.Num()`;
   `GameState.TotalPlayers = max(AlivePlayers.Num(), value from
   0x1419997b0)`.
6. Large team games: `TeamXPlayersLeft[Index] += 1`.
7. Log `Player [%s] added to alive players list (Team [%d]). Player count is
   now [%d]. Team count is now [%d].`

### 7.2 The pawn-destroyed path, `0x1419eabb0` (read)

A controller virtual (one vtable reference, no direct caller) that runs when
the controller's pawn is destroyed. On the server, for an Athena pawn whose
PlayerState is not already marked dead, when the phase is SafeZones, the
controller is `bMarkedAlive` and a timer is not pending, it asks the game
mode (slot `+0x9e0`) and logs `Removing %s from alive players, pawn was
destroyed without dying`. **The removal call after that log line is an empty
stub in the client**, which confirms D6: the call into
`RemoveFromAlivePlayers` from the pawn side was compiled out. The condition
around it is still useful: it shows the original only removed a player this
way during the SafeZones phase.

## 8. One probe run should print

Per second: `GamePhase`, `AlivePlayers.Num()`, `TeamAlivePlayers` non-zero
entries, `PlayersLeft`, `TeamsLeft`, per controller `bMarkedAlive`, the
match-ended byte, `WinningTeam`, `WinningPlayerState`, `WinningPlayerList`.

## 9. Open items

| Item | Needed for |
|---|---|
| `0x142a52de0`, `0x142a3bda0`, `0x1419f4520`, `0x141a2bff0` | exactly what the winner receives |
| `0x1428ef5b0`, `0x141963e70`, `0x141946150`, `0x1419ae460`, `0x141962af0` | end-of-match timing and the return to lobby |
| the `+0xdc0` timer callback | what happens after the post-match delay |
| `0x1419783c0` | called before every win |
| `0x14196d120` | team win (duos and squads) |
| mode tests `0x14199e430`, `0x14199e4e0` | confirm false for solo |
