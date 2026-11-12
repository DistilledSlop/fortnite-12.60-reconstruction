# D6: combat, damage, elimination, placement (what exists, what to re-create)

Date: 2026-11-11. Tools: `rpc_sweep.py`, `rd.py`, `callers.py` (new: finds
direct callers and vtable references of an address).

## 1. What exists in the client as real code

| Piece | Where | Note |
|---|---|---|
| Weapon fire, reload, charge RPCs on `AFortWeapon*` | sweep: `mixed`/`real` (`ServerReleaseWeaponAbility`, `ServerSetChargeState`, `ServerSetMuzzleTraceNearWall`, …) | firing itself goes through gameplay abilities (GAS), which is engine/plugin code we have in source |
| Ability activation RPCs (`UAbilitySystemComponent::ServerTryActivateAbility` …) | stock UE 4.25 GameplayAbilities plugin | no reconstruction needed, only the Fortnite ability classes and their Blueprints |
| Damage cues to clients: `AFortPawn::NetMulticast_Athena_BatchedDamageCues` | 0x142334fc0 | hit markers and damage numbers come from this multicast |
| `ClientReportDamagedResourceBuilding` | 0x1423727b0 | client-side receipt for harvesting (shows +wood) |
| `UFortControllerComponent_Interaction::ServerAttemptInteract` | 0x141d15810 | chests, doors, vehicles: real |
| `AFortPlayerController::ServerLoadingScreenDropped` | 0x1423abb20 | sets bLoadingScreenDropped, calls GameMode vfn +0xb20 with the controller |
| `ServerReadyToStartMatch`, `ServerSetClientHasFinishedLoading`, `ServerReturnToMainMenu`, `ServerRemoveInventoryItem`, `ServerEmote`, `ServerPlayEmoteItem`, `ServerClientIsReadyToRespawn` | sweep: real | port as is |
| `AFortGameModeAthena::RemoveFromAlivePlayers` | **0x14195e620** | real. Removes the controller from AlivePlayers, decrements TeamAlivePlayers[team], writes `GameState->PlayersLeft`, logs "removed from alive players list", then calls the win check (through 0x141937f90). Arguments: (GameMode, PlayerController, FinisherPlayerState, FinisherPawn, FinishingWeapon, DeathCause, bool). |
| `RemoveFromAliveBots` | 0x14195e410 | same for bots (AliveBots +0xde0) |
| `CheckWinConditionLastManStanding`, `StartEndGamePhase`, `HandleMatchHasEnded` | D2 card §9 | real |
| `AFortPlayerControllerZone::ClientOnPawnDied` (client side) | 0x1423cd300 | stores the death report (+0x2818), leaves vehicle, sets PendingSpectatorLocation = pawn location + 160 up, enters death camera |
| `ClientNotifyWon`, `ClientNotifyTeamWon`, `ClientSendEndBattleRoyaleMatchForPlayer`, `ClientSendMatchStatsForPlayer`, `ClientSendTeamStatsForPlayer`, `ClientSetDeathReport` | 0x1419c6120, 0x1419c5ae0, 0x1419c7970, 0x1419c7b40, 0x1419c7d90, 0x1419c7db0 | client receivers; the server only has to call them |
| `AFortPlayerControllerAthena::RespawnPlayerAfterDeath` | 0x1419ee720 | respawn modes only |

## 2. What is missing: the server death sequence

`RemoveFromAlivePlayers` has exactly two direct callers in the client: `Logout`
and a team-change handler (0x14195a3d0). The call from the death path is not
in the binary. So the code that runs on the server when a player's health
reaches zero was compiled out, and is the one large re-creation in this area.

Design (mine). Entry point: the server-side death notification of the pawn.
Find it when porting `AFortPlayerPawn`/`AFortPawn`: it is the virtual that
calls `OnDeathPlayEffects`/sets `bIsDying`; hook after it. Then, in order:

1. Build `FFortPlayerDeathReport` (KillerPlayerState, KillerPawn, DamageCauser,
   ServerTimeForRespawn, Tags = damage tags). Layout: reflection struct, 0x78
   bytes on the client (fields copied in ClientOnPawnDied above).
2. Victim `AFortPlayerStateAthena`: fill `DeathInfo` (FinisherOrDowner, bDBNO,
   DeathCause, Distance, DeathLocation, DeathTags, bInitialized = true), set
   `PawnDeathLocation`, call `OnRep_DeathInfo`. DeathCause comes from the
   damage tags (`EDeathCause`; there is a tag → cause mapping function, search
   the reflection for `ToDeathCause`).
3. Killer state (if a different player): `KillScore += 1`, `TeamKillScore += 1`,
   call `OnRep_Kills`, `ClientReportKill(VictimState)`. The kill feed on all
   clients is driven by the replicated DeathInfo.
4. Victim controller: `ClientOnPawnDied(Report)`.
5. Drop the inventory: for every entry whose definition can be dropped, spawn a
   pickup at the pawn and toss it (shared helper from the D5 card); clear the
   inventory. Also `MaterialDropOnElim_Default` modifier: materials drop too.
6. If no respawn (default playlists): `GameMode->RemoveFromAlivePlayers(PC,
   KillerState, KillerPawn, KillerWeaponDef, DeathCause, 0)`. That updates
   PlayersLeft and runs the win check.
7. Placement for the victim: `Place = PlayersLeft + 1` (after step 6), call
   `OnRep_Place`. Then `ClientSendMatchStatsForPlayer(FAthenaMatchStats)`,
   `ClientSendTeamStatsForPlayer(FAthenaMatchTeamStats{Place, TotalPlayers})`,
   `ClientSendEndBattleRoyaleMatchForPlayer(true, FAthenaRewardResult)`:
   these fill the "You placed #N" screen.
8. Report the result to the backend (`POST …/result`, D3 card §4).
9. Winner (from `StartEndGamePhase`): winner's `Place = 1`,
   `ClientNotifyWon(FinisherPawn, FinishingWeapon, DeathCause)`, the same three
   stats calls, and the GameState's `WinningPlayerState` / `WinningTeam`.
10. Spectating: the client starts its own death camera from step 4. After the
    playlist's spectate delay, set the victim controller to spectate the killer
    (`PlayerToSpectateOnDeath` on the controller, then the state change to
    spectating). `IsSpectatingAllowedAfterDeath` (GameModeZone, real) gates it.

DBNO (squads/duos only, not needed for solo): at zero health, if
`bAlwaysDBNO` or the team has others alive: set `bIsDBNO`, apply the DBNO
effect, health = `Default.DBNO.InitialHealth` (100), bleed `BleedDamage` 2 every
`BleedTick` 1.2 s; real death when it reaches zero or when finished.

## 3. Damage path (design check, not re-creation)

Damage in Athena is a GameplayEffect applied by the weapon ability, executed by
a damage execution class, which writes the Health/Shield attributes of
`UFortHealthSet`. All of those are C++ classes in FortniteGame and are in the
port list (search `functions.jsonl` for `HealthSet`, `DamageExecution`). The
re-creation risk is in the attribute set's server-only `PostGameplayEffectExecute`
(shield first, then health; clamp; death at zero). When porting it, compare
with the decompile; if the body is thin, the rule is: damage reduces Shield
first, the rest reduces Health, storm damage (`GE_OutsideSafeZoneDamage`)
ignores Shield.

Harvesting: pickaxe hit on an `ABuildingSMActor` → the building's damage
handler gives resources. Amount per hit = `AthenaResourceRates` row for the
building's `BuildingResourceAmountOverride` curve (D4 card §4), double on a
weak-spot hit; add to the inventory stack and call
`ClientReportDamagedResourceBuilding(Building, ResourceType, Amount, bDestroyed,
bJustHitWeakspot)`. Check whether the building's server damage handler
(`OnDamageServer`, not reflected) exists before re-creating: use `callers.py`
on `ClientReportDamagedResourceBuilding`'s send stub.

## 4. Tests with the vanilla client (two clients needed from step 3)

1. Shoot a wall: damage numbers appear, wall health drops, wall breaks.
2. Harvest a tree: wood count rises, weak-spot circles appear.
3. Client A eliminates client B: B sees the death screen with A's name and
   placement #2; A sees the kill feed line, the elimination count and Victory
   Royale; B's items lie on the ground.
4. Storm kills a player: death cause "storm" in the feed.

## 5. Work items

1. Port `RemoveFromAlivePlayers`, `RemoveFromAliveBots`, win check, end game.
2. Write the death sequence in §2 as one function
   `AFortGameModeAthena::HandlePlayerEliminated` (our name) and call it from
   the pawn's server death notification.
3. Port `UFortHealthSet` and the damage execution; verify §3.
4. Locate with `callers.py`: the pawn's server death virtual, the building
   damage handler, the tag → EDeathCause function.
