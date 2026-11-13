# G9: elimination, match end, lobby return and persistent results

Research date: 2026-11-13. Research only; no build or gameplay run performed.
Scope: vanilla 12.60 client, default solo, reconstructed Linux dedicated server.
Read with R3, D6, R6 section 5, and E-backend-remaining.md section E2.
**Read** below means binary/source/reflection evidence; **proposal** means a
reconstruction of missing server code; **unverified** means no original-client
runtime proof. Do not promote a proposal to original behavior or a passed gate.

## 1. Current implementation and corrections to earlier cards

- `Private/FortAthenaHealth.cpp:444` already has a re-created death sequence:
  dying guard, movement stop, DeathInfo, killer score, death RPCs, a private
  alive list, placement and three result RPCs. It also declares a winner itself.
  Empty reward/stat payloads are stand-ins; they do not implement progression.
- `Private/FortAthenaEndGame.cpp` separately implements solo winner fields,
  winner list and ClientNotifyWon, plus `-FortniteQuickEndGame` for a one-player
  diagnostic. Its log explicitly leaves end-match helper/results/return unresolved.
  Health's winner path omits WinningTeam/WinningPlayerList and sends WonMatch
  instead of preserving the elimination's finishing arguments. Consolidate these
  paths; do not add a third winner/result sender. No call to
  `FortAthenaEndGame::OnPlayerEliminated` was found in the inspected gameplay files.
- E2 is a section, not a separate E2 file. Its implementation status is stale:
  `server/gameplay_service.py`, `progression_service.py` and
  `season12_progression_data.py` already persist results and apply progression.
  No game-server result submitter was found in `FortAthena*.cpp` or
  `FortniteAthenaGameplay.cpp`. E2's automatic lobby QueryProfile claim still
  requires postmatch observation; its seasonal XP asset path is superseded below.
- R3's `0x1428ef5b0` is **EndGamePhaseStarted**, a ProcessEvent wrapper, not
  engine EndMatch: name initializer `0x1403d57b6` writes the same name global
  `0x1481962d0` used by that wrapper. Do not replace this event with an assumed
  engine call and claim an exact port. The helper's later virtual `+0x828`
  resolves to `AFortGamePvPBase` body `0x14198e1b0`; its full contract is unresolved.
- R3's corrected StartEndGamePhase entry is `0x14196cb30`.
  Use `rd.py ... --exact` for known entries: its alignment heuristic resolves
  valid small functions to unrelated predecessors. Raw disassembly confirms
  `0x1419c7d90` (team stats) and `0x1419c7db0` (death report) are distinct entries.

## 2. Authoritative death-to-results contract

The server death sequence was compiled out of the shipping client (D6).
The following orchestration is therefore a **proposal** around recovered
receivers and the read alive-list/win logic, not recovered server ordering.

1. Make death idempotent per pawn/life. Resolve victim controller/state before
   detaching/destroying the pawn. Set dying, disable movement and further damage;
   retain replicated actors long enough for death references and camera transition.
   Default solo goes directly to elimination: no DBNO, resurrection or respawn.
2. Build `FFortPlayerDeathReport` and replicate `FDeathInfo` on victim state.
   DeathInfo needs initialized=true, DBNO=false, cause, finisher/downer,
   distance/location/tags; set PawnDeathLocation. Force a state net update.
   Use existing `ToDeathCause` body `0x141a5f1c0` and real damage tags; storm
   must remain storm, not an invented killer or a blanket Unspecified cause.
3. For a different, valid killer, increment KillScore and appropriate team score,
   notify score changes, and invoke `ClientReportKill(VictimState)` on the killer
   state. Environmental death must not grant a self kill. The replicated DeathInfo
   drives the global kill feed; ClientReportKill is not a broadcast substitute.
4. Send victim `ClientSetDeathReport(Report)` and `ClientOnPawnDied(Report)` once
   each. Existing Health sends OnPawnDied first; receiver evidence does not prove
   a mandatory ordering. Keep the report identical in both; validate the combined
   flow. `ClientOnPawnDied` handles input/build/vehicle cleanup and death camera.
5. Drop eligible inventory through the shared inventory/loot implementation;
   preserve drop restrictions. This is a G8 dependency, not a second G9 inventory
   implementation. Remove the victim from the one authoritative alive roster.
6. Port R3's removal: snapshot old player/team counts, clear marked-alive/damager
   fields, decrement correct team count, derive PlayersLeft from the roster, then
   check wins with the old counts. Do not mutate GS counts in two systems.
   For ordinary solo elimination, victim place = remaining players + 1. Freeze
   total match participants from match start; Health's current `max(old,place)`
   undercounts later eliminations in matches larger than two. SecondsAlive must
   measure participation, not absolute server uptime as Health currently does.
7. Freeze one result per account/match: place, kills, actual statistics, duration,
   legitimate XP/accolades and the stable backend match/session identity.
   Send team stats and match stats, then end-BR reward result after award data is
   known. This sequence is a proposal; all three are separate client receipts.
   Persistence and receipts must agree; success=true with fabricated rewards is
   not acceptable. Keep RPCs on the owning controller before tearing it down.
8. If a win occurred, finalize the survivor through the single winner path below.
   The eliminated player's individual results need not wait for the whole match.
   If nobody remains, record the final death and explicitly terminate the empty
   server match without inventing a winner (local cleanup policy, not a solo win).

### Client-facing evidence and payloads

| Receiver/state | Read evidence and requirement |
|---|---|
| `ClientOnPawnDied`, `0x1423cd300` | Copies report; resets vehicle/input/build state; pending spectator location = pawn location +160 Z; non-dedicated UI notification gated by bNotifyUI. |
| `ClientSetDeathReport`, `0x1419c7db0` | Stores report; positive respawn time can start respawn only under GS permission and outside EndGame. Do not promise a respawn for solo. |
| `OnRep_DeathInfo` | `generated/decomp_improved/AFortPlayerStateAthena/OnRep_DeathInfo.c`: initialized gate, tags and killer/team/local-killer feed formatting. Server OnRep calls alone do not deliver client replication. |
| `ClientReportKill` | Reflected exec `0x142a8d5c0`; owner receipt, alongside replicated scores. |
| `ClientNotifyWon`, `0x1419c6120` | Sets winner flag and schedules win presentation using `0x1419922b0(GS,1)` (minimum 0.01 s). Not just the Place repnotify. |
| `ClientSendTeamStatsForPlayer`, `0x1419c7d90` | Reads controller MatchReport at +0x3318; tail-calls `0x141b5d5d0`, stores Place/TotalPlayers and readiness, processes delegates. Null report drops the receipt. |
| `ClientSendMatchStatsForPlayer`, `0x1419c7b40` | Updates heartbeat, one-time stats work, forwards to MatchReport if present. |
| `ClientSendEndBattleRoyaleMatchForPlayer`, `0x1419c7970` | Forwards success/reward to MatchReport; reward-dependent profile work exists. Does not itself persist anything in our backend. |

Payload layouts come from reflected structs and generated headers, not guessed ABI:

- `FFortPlayerDeathReport` is 0x78 bytes in the original: respawn/resurrect
  times at 0/4; lethal damage 8; killer state/pawn 16/24; killer health/shield
  fractions 32/36; weapon 40; damage causer 48; dropped-backpack/notify-UI flags;
  Tags 64; view location 96 and rotation 108. Use actual lethal damage and real
  dropped-backpack status; current Health hard-codes these. Solo absent respawn
  times currently use -1; the receiver may normalize respawn time to zero.
- `FAthenaMatchTeamStats`: Place and TotalPlayers, both int32.
- `FAthenaMatchStats`: StatBucket, MatchId, MatchEndTime, MatchPlatform strings;
  `int32 Stats[20]`, WeaponStats, XPStats, FactionTag. Do not guess that Stats[0]
  means kills, or keep `StatBucket="Default"` as verified protocol. Read the
  report consumers before populating numbered slots; unknown mappings block
  claims that the detailed results screen is complete.
- `FAthenaRewardResult`: LevelsGained, BookLevelsGained, TotalSeasonXpGained,
  TotalBookXpGained, PrePenaltySeasonXpGained, XpMultipliers, Rewards and
  AntiAddictionMultiplier. A zero-initialized structure is not evidence of a
  legitimate zero-XP match; resolve neutral multiplier defaults and award logic.
- Use generated C++ declarations for call parameter order. Reflection metadata
  collection order and decompiler register guesses are not a C++ signature.
  Reliable controller RPC order does not guarantee that GameState/PlayerState
  actor updates arrive first. Verify reference resolution, MatchReport readiness
  and the resulting client screen; a server-side OnRep call proves none of them.

### Spectating after death

Read: `AFortGameModeZone::IsSpectatingAllowedAfterDeath`, `0x141ea2510`,
returns bAllowSpectateAfterDeath. Constructor evidence in
`generated/agents/ctor-defaults/ctor_defaults.json` under `by_class`:
Athena allows spectating (`0x14192e73a`); Zone first/repeat delays are 5/2 s
(`0x141e89e95`, `0x141e89e9f`). Playlist/runtime overrides remain to be checked.
The death camera is not proof of server spectating.

Proposal: after permitted delay, pick a live eligible killer, otherwise another
eligible survivor; set PlayerToSpectateOnDeath, move controller into Spectating
and set its view target through recovered/engine APIs. With no survivor retain
the death/results view. Cancel/reselect when target dies or disconnects; reject
live-player, foreign-team-policy and invalid-target requests. Read
`ServerSpectatePlayer` exec `0x142a70f50` and `ServerSpectatePlayerState` exec
`0x142a71050` before re-creating their currently stubbed implementations.
Do not destroy the victim controller/PlayerState while it owns results RPCs.

## 3. Victory, EndGame and timed departure

Read sequence (R3): removal -> wrapper `0x141937f90` -> win check
`0x141937590` -> solo `StartEndGamePhase(0x14196cb30)` -> end helper
`0x14193df50`. The gate `0x14194e680` permits Aircraft/SafeZones only and
rejects an already-ended match. Solo victory requires **old >=2 and new ==1**,
MaxTeamSize==1 and no forced team-score placement/special-mode override.
RequiredPlayers=1 changes readiness; it does not change this win condition.

Winner: set place 1 through the placement path `0x1419f4520` ->
`0x141a5aa60` -> setter `0x141a5a880` (setter side effects still need review);
finalize survival accounting and team place; send ClientNotifyWon with original
finisher pawn/weapon/death cause. Set WinningTeam, WinningPlayerState and unique
WinningPlayerList entry (reflected `FFortWinnerPlayerData{PlayerId}`), force GS
update. Send the winner the same consistent three result receipts. R3 also
finds `ClientFinishedInteractionInZone` here; its purpose remains unresolved,
so it is not a substitute for any result RPC. Team win is a separate original
path `0x14196d120` using ClientNotifyTeamWon/Lost; do not claim duos/squads parity.

End helper: fires EndGamePhaseStarted, pauses storm via `0x141963e70`, does
survivor/special-actor work, invokes `+0x828`, and sets nonreflected ended byte
`+0x1088`. Generated storage lacks many original private fields: implement
equivalent state in the hand-written per-world match owner, not raw offsets.
Recover the transition into HandleMatchHasEnded before claiming a literal port;
if engine EndMatch is used as the bridge, label that bridge a reconstruction.

`HandleMatchHasEnded`, `0x14194b750`, calls Zone parent, then:

1. `0x141946150` selects delay. Constructor defaults: NoIGMM=60 s
   (`0x14192dec9`), IGMM=120 s (`0x14192ded3`) plus GS helper `0x1419920d0`.
   Runtime option `0x1426f5b80` selects branch. Initialize recovered defaults;
   do not leave delay zero because generated classes lack native constructors.
2. `0x1419ae460(GS,Now,Delay)` writes EndGameStartTime and
   EndGameKickPlayerTime=Now+Delay and broadcasts its delegate. Replicate the
   kick deadline with its OnRep; use synchronized server world time.
3. Schedules timer on original +0xdc0, callback **OnEndGameKickPlayers**
   `0x141952db0` (confirmed exec `0x1428f0650` jumps there), sets GamePhase
   EndGame through `0x1419ae750`, clears gameplay timers/session hooks.
   GamePhaseStep EndGame=11 is a separate, nonreplicated field; the vanilla
   client derives UI state. Current MatchFlow's server step assignment is
   explicitly a stand-in. A step assignment alone sends no phase transition.
4. Callback, when FortGameSession exists, walks eligible Athena controllers
   and sends **ClientEndGameKick** (`0x142a3bcb0`, name global 0x1481aef50,
   initializer `0x140417f1d`), then calls shutdown helper `0x1419602e0`.
   That helper invokes Zone virtual `0x141eb41a0` and schedules further delayed
   cleanup, at least 5 s with configured jitter. Exact final process-exit
   callback remains unresolved; do not kill the process at the kick deadline.

Read: ClientEndGameKick exec `0x142a44650` dispatches virtual +0x14a0 to
`0x1423713b0`; that clears a controller delegate and invokes the return-reason
virtual +0x9e8. On Athena this is Zone `0x1423cdf40`, which performs cleanup
then tail-calls Fort `0x142373220`, ClientReturnToMainMenuWithTextReason.
Sending this client RPC through its generated wrapper works with the vanilla
receiver; reconstructing its UI implementation on the dedicated server is unnecessary.

## 4. Clicking Return to Lobby

Read client `ReturnToMainMenu`, `0x1423a8e90`: clears inventory UI state,
marks disconnect reason, sends **ServerReturnToMainMenu** through
`0x142a3f3d0`, and starts a local timeout if needed. This is not a new HTTP
"return to lobby" endpoint. Original server body `0x1423ac090` starts its
timeout, touches session ownership, logs `FortPC::ServerReturnToMainMenu()`
and invokes virtual +0xa58. This resolves to engine RPC wrapper
`0x1447baa50`, **ClientReturnToMainMenuWithTextReason** (name global
0x1483612b8, initializer `0x14058c7ad`); the header-model label in
vtable_lookup incorrectly calls this slot UpdateCameraManager. Trust binary
name initialization, not that low-confidence model label.

The original client receiver `0x142373220` clears the timeout and calls its
GameInstance/menu cleanup route; Zone `0x1423cdf40` adds cleanup first.
Stock UE `Engine/Private/PlayerController.cpp:1819` likewise returns through
GameInstance, with disconnect fallback. Use this inherited reliable RPC with
an empty normal-return reason; do not ClientTravel the player to Apollo/lobby
map URLs. The client owns the frontend transition.

Generated `ServerReturnToMainMenu_Implementation` is currently a stub. Port
the retained server body or bind a scoped override using InstallRpcOverrides,
with result flush, one-player departure and roster/session cleanup. Do not
disconnect the other clients or shut a live match down when one loser leaves.
ReturnToMainMenuTimeoutDelay is 5 s on base PC (`0x14236619e`), overridden to
10 s on Athena (`0x1419b8843`); getter `0x142385220` reads it. Timeout body
`0x1423a92bd` handles authority pawn/session cleanup and invokes the return
RPC. Port required Athena timeout overrides before claiming full parity.

## 5. Backend: existing contracts and evidence limits

| Exact route | Evidence/status |
|---|---|
| `POST /local/gameserver/register` | Existing project stand-in, bearer secret; address, port, playlists[], maxPlayers, buildUniqueId; optional beaconPort. Returns sessionId and encryptionKey. Never log key/authorization. |
| `POST /local/gameserver/<sessionId>/state` | Existing stand-in; phase, numPlayers, joinable; optional connectedPlayerIds. Heartbeat must remain within 30 s. Stop admissions when ending. |
| `POST /local/gameserver/<sessionId>/result` | Existing authenticated stand-in in gameplay_service.py:54; this is NOT an Epic client endpoint. Account must have been admitted. |
| `POST /fortnite/api/matchmaking/session/<sessionId>/join` | Existing client join route/admission check. Keep this same session identity for results. |
| `POST /fortnite/api/game/v2/profile/<accountId>/client/QueryProfile?profileId=<id>&rvn=<revision>` | Existing MCP route. Login common_public request observed at generated/runs/20261113-play-run-26/client.log:3455; athena/common_core refresh after this match's return is unverified. |
| `POST /statsproxy/api/statsv2/query` | Observed HTTP 200 in original-client lobby, server/EVIDENCE.md:786-791, not observed postmatch/Career. Body/parser evidence: 0x140bb28a0, 0x140bc9c20, 0x140bb35d0, 0x140bca120. |
| `GET /statsproxy/api/statsv2/account/<accountId>` | GET contract inferred. Binary literal /api/statsv2/account/ at 0x145ad64e0, builder 0x140b4a3e0, xref 0x140b4a58e; endpoint_evidence.json:2797. Existing inferred handler is not Career acceptance proof. |

Result body: accountId, placement (1..100), kills (0..99), xp; optional bookXp,
accolades (known original row -> count), playlist, endedAt (UTC ISO8601).
Accolades can derive xp; an explicit xp must equal their sum. Existing service
atomically stores `(session_id, account_id)` canonical payload/receipt and
profile changes in profiles.sqlite3. Receipt: sessionId, accountId, recorded,
xpAwarded, bookXpAwarded, endedAt (Unix seconds), profileUpdate (MCP envelope).
Exact replay does not re-award; conflicting replay fails. Retain the immutable
payload, including timestamp, across retries. Client RPCs cannot authorize XP.

**Ordering defect to address:** gameserver_service.py:81-85 removes departed
players from its admission list. First result submission afterward fails 403;
only an already-recorded exact replay bypasses the membership check. Prefer a
durable admitted-participant ledger separate from the connected roster, including
authorization after backend restart. Until that is implemented, obtain result
receipt before publishing departure; do not present this timing workaround as
reliable crash recovery. The current session registry is process-local.

Real season XP curves and accolade values are already embedded with provenance
in season12_progression_data.py. They do not establish accolade trigger rules.
Persist real awards and return matching reward deltas; do not invent kill or
placement XP, double-award via MCP, or report a made-up neutral multiplier.
Existing stats counter names/playlist/platform aggregation are inferred; verify
against actual Career requests/display. Do not return arbitrary empty objects
and call Career complete. Profile persistence, results UI and Career are three
separate acceptance checks.

## 6. Ordered work packages (single Native promotion/build owner)

Every package inherits the research limits above. Tests below are future work,
batched at the integration checkpoint; this card does not authorize this worker
to run them. Preserve ongoing collaborators' edits; do not edit Generated files.

| WP / owner files | Implementation order and done criterion | Assets / validation |
|---|---|---|
| **G9-1 Death and roster integration**: existing FortAthenaHealth.cpp; new Private/FortAthenaEliminationFlow.{h,cpp}; FortAthenaEndGame.cpp | Port R3 add/remove 0x141933cb0/0x14195e620, wrapper/gate/win check 0x141937f90/0x14194e680/0x141937590. Use one roster, initial participant count, one life-finalization guard; preserve death cause/report/drop behavior; remove duplicate winner logic. Done: exactly one removal, score award and result snapshot per elimination/logout, no solo DBNO or self kill. | Existing Playlist_DefaultSolo and death-tag mapping through GameData must be loadable; reuse G8 drop assets. Test T1/T3: death/report/feed and count/placement logs agree. |
| **G9-2 Individual receipts and spectating**: new Private/FortAthenaMatchResults.{h,cpp}, FortAthenaEliminationFlow.cpp; generator inputs for recovered bodies only if needed | Read consumers at 0x1419c7d90 -> 0x141b5d5d0, 0x1419c7b40, 0x1419c7970; recover Stats[20]/bucket semantics before filling them. Review 0x141a5a880 place setter; implement one receipt sender, shared victim/winner snapshots. Read spectate execs 0x142a70f50/0x142a71050 to their true bodies, permission 0x141ea2510; initialize 5/2 s delays. Done: correct placement, detailed real stats/rewards and valid live-target camera; no receipt dropped before MatchReport initialization. | No server UI staging. Test T1/T3 screenshots plus proposed G9: result/spectate logs; detailed stat mapping is a blocker, not a zero-filled completion. |
| **G9-3 Win and match-end lifecycle**: FortAthenaEndGame.cpp/.h, FortAthenaMatchFlow.cpp | Port 0x14196cb30, 0x14193df50 and resolve +0x828 (0x14198e1b0)/engine-ended bridge; then 0x14194b750, 0x141946150, 0x1419ae460 and 0x141952db0. Reuse phase setter, pause storm, set winner fields/list, finalize winner receipts; initialize 60/120 s defaults and session/match guard. Read final delayed shutdown callback behind 0x1419602e0 before implementing process exit. Done: one win, EndGame deadline and orderly kicks; no damage/late duplicate victory afterward. | Native logic, no new UI packages. T2 is diagnostic only; T3 proves real victory, T4 proves deadline. |
| **G9-4 Durable results bridge**: new Private/FortAthenaBackendResults.{h,cpp}; integrate gameplay hooks; server/gameserver_service.py, gameplay_service.py, progression_service.py only where needed | Reuse registration/admission/session identity; implement authenticated HTTP async submit/retry with immutable results and surfaced failures; retain durable participant eligibility before connected-roster removal. Use existing progression transaction/receipts; recover actual event-to-accolade rules and reward payload mapping. Done: one committed result per account, correct profile deltas, exact retries after disconnect/restart, conflicting replay rejected. | Python already embeds /Game/Athena/Balance/Seasons/Athena_Season12_XP, Athena_Season12_ExtendedXP and /Game/Athena/Balance/DataTables/AthenaAccoladeXP. No new UE staging for those unless native award lookup is chosen; then stage .uasset/.uexp and dependency closure. T1/T3 profile comparison plus focused existing persistence/replay tests. |
| **G9-5 Return and cleanup**: new Private/FortAthenaReturnToLobby.{h,cpp}; gameplay override installer, EndGame timer, backend roster hooks | Port retained 0x1423ac090 timeout/session work, using generated ServerReturnToMainMenu UFunction override and inherited ClientReturnToMainMenuWithTextReason; read 0x1423a92bd/Athena timeout override. Reuse 0x1423713b0/0x1423cdf40 client semantics, preserve live peers; publish ended/nonjoinable and drain results before teardown. Done: button return and automatic deadline both reach usable lobby; no lost results or disconnect of another active player. | No server frontend assets; original client already owns UI/maps. T1/T3 click test, T4 automatic return; capture HTTP rather than assuming refresh. |
| **G9-6 Acceptance**: existing run tools and focused backend tests; fix only affected implementation files | One combined server build/stage at agreed checkpoint, then T1-T4; retain source/build/run ids and useful failure windows under generated/builds and generated/runs. Investigate crashes with debugger. Done: evidence for death, stats, victory, button/timer return, persistent profile, two clients and a second match; no placeholder payload or diagnostic-only win counted as G9. | Reuse staged Apollo/solo gameplay closure. No separate client build for vanilla-client testing; no UI asset conversion or unrelated retained suite. |

## 7. Concrete original-client integration tests (not executed)

All tests use `tools/vanilla_play_run.sh`, duration 1800, `FN_RENDERER=dxvk`,
`FN_LP_NUM_THREADS=4`, `FN_SERVER_MAP=/Game/Athena/Apollo/Maps/Apollo_Terrain`,
`FN_SERVER_URL_OPTS='?RequiredPlayers=1'` for single client (2 for two clients),
and `FN_SERVER_ARGS='-FortniteSkipUnconvertedLegacyShaderMaps -FortniteTolerateNeedLoadDependencies -FortniteSkipEngineClassNetChecksum -FortniteWarmupSeconds=20'`.
Keep run server.log/client.log/runner.out/meta.json/shots and redacted HTTP
request paths/profile deltas under that run. Do not archive credentials.

Shared exact login action prefix (existing 1280x720 flow):
`0:wait,UI.State.Startup.SubgameSelect,900 20:click,640,592 0:wait,ShouldShowBeginningOfSeasonDialog,600 25:click,1144,473 0:wait,Join.succeeded,60 0:shot`.
Wait actions reset the schedule base; use unique shot times to avoid overwrites.
Emit the following **new proposed** G9 log tokens with the implementation:
`G9: eliminated`, `G9: results-sent`, `G9: spectating`, `G9: winner`,
`G9: result-committed`, `G9: return-request`, `G9: kick-deadline`.
Include match/account correlation, place/participants and result identity;
log RPC send and backend receipt separately. These tokens do not exist today.
Before a two-client run, prepare its separate prefix while the first prefix is
idle with `porting/tools/second_client.sh --prepare`; copying a busy source is
refused. Keep second-client logs/screenshots in the helper's `<run>/client2/`.

- **T1, one-player storm death:** append
  `0:wait,G9:.eliminated,1200 3:shot 0:wait,G9:.results-sent,60 8:shot`.
  Stay at the forced landing location and let the real storm kill the pawn;
  require health/storm log progression, not just the proposed G9 marker. Expect
  place #1 of 1, zero kills and zero survivors, **no Victory Royale**, because
  the old>=2 condition fails. This is an individual-result/empty-match test.
  If the real storm does not reach/kill the stationary player within timeout,
  record failure and adjust position/run length; do not silently inject damage.
  Screenshot must show storm death and placement. Click the actually labeled
  Return to Lobby button, record its coordinates in the run's FN_ACTIONS replay,
  then capture lobby and subsequent request/profile deltas. This UI's return
  coordinates have not been observed: do not invent an automated click.
- **T2, one-player diagnostic victory:** add `-FortniteQuickEndGame` and append
  `0:wait,EndGame:.solo.winner,900 15:shot`. Require winner place/list/team,
  victory screenshot and coherent results. Label it forced victory: it tests
  presentation/lifecycle only and cannot validate last-man-standing or G9 alone.
- **T3, two real clients:** set RequiredPlayers=2, start the first run with the
  login prefix plus `0:wait,G9:.winner,1200 15:shot`; start the second using
  `tools/second_client.sh <run> --username LocalPlayer2 --renderer dxvk --actions`
  with the same login prefix plus
  `0:wait,G9:.eliminated,1200 8:shot 0:wait,G9:.spectating,60 18:shot`.
  First perform a real storm elimination with A inside/B outside; require B
  #2 of 2, A #1 and victory, no killer credit for storm. Then repeat in a new
  match with working G8 combat, A kills B: kill feed, A kills=1, B sees A/cause,
  death camera then spectating, B's drops, both result receipts. Click B's real
  Return to Lobby button and ensure A stays connected; both profiles persist
  distinct correct results. If shooting is unavailable, storm tests proceed
  but killer credit/combat acceptance remains explicitly blocked by G8.
- **T4, automatic departure and persistence:** repeat T3 without clicking return;
  append `0:wait,G9:.kick-deadline,300 20:shot` on each client. Verify deadline
  equals recorded start+selected delay, ClientEndGameKick/normal return logs,
  lobby screenshot and no premature process kill. Restart backend, QueryProfile
  and open Career; compare actual requests, stored result/profile revisions and
  displayed values. Replay the identical server result to prove no extra award;
  then queue a second match to prove session cleanup, not merely a disconnected
  screen. Runtime logs to seek include `FortPC::ReturnToMainMenu()`,
  `FortPC::ServerReturnToMainMenu()` and
  `FortPC::ClientReturnToMainMenuWithTextReason()`; timeout fallback alone is
  not proof that the server handled the normal button path.

Outstanding parity blockers are explicit: numbered match-stat semantics,
reward multipliers/accolade triggers, MatchReport readiness timing, exact final
shutdown callback, Athena timeout overrides, postmatch profile refresh and
Career schema/display. These are bounded prerequisites inside G9-2..6, not
permission to fill unknown behavior with defaults. Screenshots plus actual
backend/profile evidence are required before G9 can pass.
