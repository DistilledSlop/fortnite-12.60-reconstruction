# Gameplay gap analysis, drop to win (2026-11-15)

Scope: what is still missing between "client joins" and "player wins and returns to lobby". Built from the server logs of the
2026-11-15 22:49-23:0x client run (`generated/chrono-server-logs/`), the generated/native sources, the retail-binary RPC sweep
(`tools/rpc_sweep.py`) and the existing plans (`server_systems/G8-combat-plan.md`, `G8-loot-plan.md`, `G9-match-end-plan.md`,
`D5`, `D6`, `R3`). No client was run for this analysis and nothing here is client-verified.

## State labels used below

| Label | Meaning |
|---|---|
| **confirmed** | the user saw it work |
| **pushed, untested** | code built and pushed, never seen working. `eacae928` (chest pass 2), `b5d7190b` (GetItemGuid / quickbar / pickup speed / interact fallbacks), `c7ab61b1` (time-discrepancy config, GC log) |
| **never exercised** | code exists or content exists, no log shows it running |
| **absent** | no code; retail behaviour lives in native code we do not have |
| **known broken** | user reported failure and no fix has landed |

No log shows a quickbar slot being assigned before `b5d7190b`. Everything about guns after pickup is therefore **never exercised**,
not "working" and not "broken".

## Evidence on shooting (the biggest gap)

* `Generated/UFortGameplayAbility_RangedWeapon.cpp` is empty; `GA_Ranged_GenericDamage` is a thin Blueprint on that native parent. Retail
  activation `0x14161b540` rejects Blueprint activation and requires the native ability bound to a ranged weapon (G8 plan section 3). Fire, trace, ammo
  consume, spread and the effect container are native code we do not have.
* The damage chain is absent: `UFortDamageFormulaExecutionCalculation.cpp` is 3 lines, `UFortHealthSet.cpp` holds property declarations only,
  and nothing implements `PostGameplayEffectExecute`, which is what retail uses to turn `Damage`/`ShieldDamage` meta attributes into health
  changes (`0x141bc11d0`, `0x141bbd020`, `0x141bbfa90`; Regen set `0x141bc15d0`, `0x141bbebf0`). `FortAthenaHealth.cpp:460` only listens for health
  attribute change; its `ApplyDamage` is our own non-GAS path that nothing on the ranged route calls.
* The only `Combat:` lines in the last log are 8 `Combat: Grant` for the pickaxe melee ability (`GAB_Melee_ImpactCombo_Athena`, `input=-1`, specs 32-39). There is no
  grant for any ranged ability, no `TargetDataAccepted`, no `DamageResolved`.
* `FortHarvestFlow.cpp` hooks `UAbilitySystemComponent::FortServerTargetDataReceived` (engine patch) and returns at line 80 unless the ability class name contains
  "Melee". The hook is generic, so ranged hit data very likely reaches it too (the client sent `ServerSetReplicatedTargetData` 31 times in the
  last run, all melee). Unverified: whether the server ability instance for a ranged ability stays active long enough for the data to be consumed,
  because our native ranged ability is empty. Decision: **extend the hook, do not port the ability class first**. Step 1 is a log-only branch
  for non-melee abilities (`Combat: RangedTargetData weapon=... hit=...`). That one run tells us if hit data arrives. Step 2 applies server-computed damage
  through the existing `ApplyDamage`/elimination path (never the client amount), with ammo decrement and cadence checks from the weapon stat row.
  Porting `FortGameplayAbility_RangedWeapon` (G8-C3/C4) is the long-term, exact-parity route; the hook is the shortest route to a hit that does something.
* RPCs the client sent that land in empty/unhandled server code: `ServerReleaseWeaponAbility` x21 (retail "mixed", not rebound), `ServerSetAutoEquipBetterItems` x1,
  `ServerModifyStat` x1, `ServerSendAimbotDetectionStatus` x6, `ServerThankBusDriver` x5. Auto-equip-better and fire-release matter for guns.

## RPC coverage (from evidence, not guessed names)

`tools/rpc_sweep.py Server` over `generated/thunk-table/final.json`: 359 Server RPCs; 99 are empty stubs in the retail client (the server code was stripped),
98 plain stub + 1 stub-base-with-real-override. **17 of the 99 are re-created** in `Private/`: aircraft jump, quickbar ack, inventory drop, building edit/repair/upgrade,
material selection, execute-inventory-item, handle-pickup (+swap variants), reset ASC, zipline state, suicide, set-in-aircraft. Not re-created and gameplay-relevant for BR:
`ServerCombineInventoryItems`, `ServerSpotActor`, `ServerReviveFromDBNO`, `ServerTeleportToReticle`(dev), `UFortControllerComponent_Interaction::ServerNotifyStartLongUse/EndLongUse`
(hold-to-interact such as revive, vending, supply drops), `UAthenaMarkerComponent::ServerAddMapMarker/RemoveMapMarker`, `AFortPlayerController::ServerAddPawnMovementInput`,
`AFortPlayerPawn::ServerCyclePart` (cosmetic), `AFortProjectileAthena::ServerNotifyEndOverlap` (grenades/projectile), `AFortPawn::ServerTeleportNearLocation`.
The remaining ~60 are creative, outpost/STW, PvE or dev cheats and can be ignored for BR.

## Missing Blueprint natives (not just RPCs)

The loader logged 100 "Skipping import Function" names; 86 are `/Script/FortniteGame`. The static header scan (function name declared as `UFUNCTION` in
Generated/Public headers, including inherited) flags **all 86**, so the scan and the runtime agree. When a cooked Blueprint calls an undeclared native, our patched
`SkipCallToUndeclaredNativeFunction` (`ScriptCore.cpp`) evaluates and discards the parameters, leaves the result untouched and continues: it is a **silent no-op returning
zero/null/false, not a crash**, and it warns once per calling Blueprint (57 such warnings in the last log). So "medkit does nothing" is the realistic failure, not a server crash. (Exception: native C++ that
dereferences a null result.) Fatal only if the BP then uses the null result in a way that crashes in another native.

Server-gameplay natives that silently no-op today, by effect:

| Group | Missing natives | Visible effect |
|---|---|---|
| Item grant/remove | `FortKismetLibrary::GiveItemToInventoryOwner`, `K2_RemoveItemFromPlayer`, `K2_RemoveFortItemFromPlayer`, `K2_SpawnPickupInWorld`(+WithClass), `K2_GetItemQuantityOnPlayer`, `PickLootDrops`, `CreateItemEntry`, `CreateTossAmmoPickupForWeaponItemDefinitionAtLocation`, `FortPlayerController::BP_FindItemInstancesFromDefinition`, `BP_FindExistingItemForDefinition`, `BP_GetInventoryItemWithGuid` | consumables and BP-driven gives/removals do nothing: medkit/shield potion/bandage do not consume or heal, vending machines and BP loot spawners give nothing |
| Damage / effects | `FortAbilitySystemComponent::BP_FortApplyGameplayEffectToTarget`, `MakeFortEffectContext`, `FortAbilitySystemBlueprintLibrary::GetFortAbilitySystemComponent`, `FortKismetLibrary::MakeGameplayEffectContainerSpecFromAbility`, `ReplaceRangeInGameplayEffectContainer`, `SetCanBeDamaged`, `IsCriticalHit`, `IsFatalHit`, `ApplyGlobalEnvironmentDamageToActor` | BP abilities that apply effects through these (grenades, traps, many consumables) apply nothing |
| Projectiles / tasks | `FortAbilityTask_PlayMontageWaitTarget::*`, `FortAbilityTask_SpawnProjectileAndWait::*`, `FortAbilityTask_WaitTargetSelection::WaitTargetSelection`, `FortKismetLibrary::SpawnProjectile`, `NotifyProjectileThrown`, `CalculateProjectileTrajectorySpline*` | the task node returns no task; the BP never waits, so the ability ends or hangs. This is why melee needed the target-data hook; every throwable/launcher ability has the same problem |
| Team / queries | `FortKismetLibrary::GetActorTeam`, `OnSameTeam`, `GetTeamAffiliation`, `GetFortPlayerControllerFromActor`, `GetGameStateAthena`, `GetServerWorldTimeSeconds`, `GetClosestActorFromArray`, `CheckLineOfSightToActor`, `FortPlayerStateAthena::IsInSquad`, `FortGameStateAthena::GetSquadMemberPlayerStates` | friend/foe checks return 0: everything looks like team 0, so "same team" branches misfire |
| Pawn state | `FortPawn::IsDead`, `FortPawn::ForceKill`, `FortGlobals::IsInAthena`, `FortPlayerPawn::BP_ForceOpenParachute`, `BeginSkydiving`, `PostResIn`, `FortWeapon::K2_ConsumeAmmo`, `HasInfiniteAmmo`, `WeaponDataIsValid`, `GetWeaponDataGameplayTags` | `IsInAthena` returning false changes which branch weapon/pawn BPs take; `IsDead` false everywhere; BP-driven ammo consume is a no-op |
| Building | `BuildingActor::BlueprintDie`, `SetHealth`, `HasHealthLeft`, `BuildingContainer::BP_SpawnLoot`, `SetSearchLootTierGroup`, `FortKismetLibrary::OpenActor`/`CloseActor`, `SpawnBuildingGameplayActor`, `BuildingStructuralSupportSystem::*` | BP death hooks and BP loot spawn skip. `FortPickup::GetItemEntry`, `TossPickup` skip |
| Local / cosmetic | `*Local` cues, force feedback, `TriggerFeedbackEvent`, sound indicators, Niagara, analytics, mesh/material setters, `FortGameplayCueFunctionLibrary::ExecuteGameplayCueLocal`, widget/HUD | no server impact; safe to ignore |

The static scan counts per area (gameplay members/statics): player abilities 17/14, weapons 62/17, props/containers/buildings 35/23, consumables+traps+throwables 109/35, vehicles 13/5, core pawn/game 11/7. Treat consumables/throwables as one block: they share the
effect + task + give/remove natives above.

## Collision and movement evidence (corrects an earlier statement)

The log does contain physics-data evidence for the "collision is still messed up" complaint, which I earlier called evidence-free:
* 514 `PopulatePhysXGeometryAndTransform(Convex): ConvexElem is missing ConvexMesh` and 257 pairs of `ForeachShape(Convex): ScaledElem[n] invalid`.
* 257 `Original Chaos collision for ... SplineMeshActor_*.SplineMeshComponent0.BodySetup_0: convex cooking failed` (per-terrain-sublevel spline meshes).
These are shapes the server cannot build, so server-side traces and the server's own movement collision silently skip them (roads/rails/spline meshes; some POI convex hulls). That can
yield "falls through / walks through" on the server and a correction on the client where the client has the full shape. Not proven for any specific spot; next step is a one-off `CollisionAudit:` that, for each
streamed level, lists static mesh actors whose body setup has zero convex/trimesh shapes, then compare with what the user is hitting.
Pickup root (corrected): the 2,457 `FortPickupAthena ... natively added scene component(s) ... none set as RootComponent - picking one arbitrarily` warnings come from engine
`FixupNativeActorComponents`. The generated `AFortPickup` constructor creates only one scene component (`CollisionCylinder`; the link, projectile and water components are not scene components), so
the "arbitrary" pick is the capsule. The root is therefore never null, my `if (!GetRootComponent())` branch in `InitializeSpawnedPickupActor` never runs (harmless), and there is no root defect; the warning is noise.
Possession: 42 `ClientRetryClientRestart` + `ServerCheckClientPossessionReliable` pairs, all between world time 396.5 and 398.4 s (about 2 s burst), well before the aircraft/skydive phase
(`ServerSetSpectatorLocation` ran 396.5-496.1 s, i.e. warmup spectate). So it is the pre-bus pawn-less period retrying restart, not a mid-game possession loss. Low priority.

## Test target

`-FortniteSpawnTestPawn` (`FortniteAthenaGameplay.cpp:278`) only spawns the `PlayerPawn_Athena` Blueprint once at start to reproduce its spawn crash; it is not a target dummy. A solo tester currently has nothing to shoot or be eliminated by except the storm and fall damage. A damageable
dummy needs bots (`FortServerBotManagerAthena::SpawnBot` is also a missing native) or a debug pawn; see "Choke points".

## Walk in player order

State column uses the labels above.

| Phase | State | Notes |
|---|---|---|
| Join / login / loading | confirmed (user reports joining, vanilla client, 0 checksum errors) | LOADING screen issue from G5 handoff is separate |
| Warmup (pre-bus) | log-observed | 42 possession retries at ~397 s harmless; spectator location RPCs run |
| Bus | log-observed (skydive phase and landing appear in the last log; not individually user-confirmed here); `ServerThankBusDriver` x5 unhandled (cosmetic) | |
| Skydive / glide | known broken (user: "cannot move until glider deployed" earlier, no new log); parity mode 3/4 uses stock PhysFalling | needs a run with skydiver log lines (`MatchFlow: skydiver`) |
| Landing / fall damage | never exercised (`FallDamage:` code exists) | |
| Harvest (pickaxe) | confirmed (user: breaking works) | resources/weak spot not user-verified in detail |
| Build / edit | pushed earlier, partly never exercised: 9 `ServerCreateBuildingActor`, 3 `Begin/EndEditing` handled; no support/structural system (`BuildingStructuralSupportSystem::*` missing), placement rules, `ServerSpawnDeco` absent | |
| Containers (chest/ammo box) | pushed, untested (`eacae928`, `b5d7190b`) | 20k lines of LootRoll/LootData/LootSearch logs per run (19,788 in last log): reduce |
| Other searchables | known broken, cause unknown | non-container branch fix is pushed, untested; Faction chest has no tier group (logs once) |
| Pickup / inventory | pushed, untested | root cause GetItemGuid; slots never observed assigned before this |
| Equip a gun | never exercised | B_Ranged_Generic imports missing natives (`FortGlobals::IsInAthena`, `FortWeapon::K2_ConsumeAmmo`, `HasInfiniteAmmo`): they silently no-op, so equip may succeed with wrong branches |
| Shoot / reload / ammo | **absent** | see shooting section; `ServerReleaseWeaponAbility` not rebound |
| Damage / health / shield | absent for GAS (non-GAS `ApplyDamage` exists) | no `PostGameplayEffectExecute`, no damage execution |
| Heal (medkit/bandage/shield) | absent | give/remove natives missing, effect natives missing |
| Throwables / traps / vehicles / consumables | absent | same native group |
| Storm | confirmed damage code, never user-verified at end circle; no phase-by-phase check | `GC: collection took X ms` log will show the 61 s stall cause |
| Die | never exercised: `HandlePlayerEliminated` exists, drop-all-on-death absent, spectate-after-death absent, DBNO choice retained | R3/G9 lay out the contract |
| Win / end | solo-only `BeginSolo` in `FortAthenaEndGame.cpp`; team/duo/squad end, victory sequence, results screen, Return to Lobby unverified | G9 |
| Return to lobby | never exercised | backend contracts in G9 section 5 |

## Choke points, in order

1. **One user run to settle the fix-pushed items** with the new build: chest search + sound + searched state, pick up a gun, check slot appears and the quickbar keeps it,
   and keep the `GC:`/`LootSearch:`/`no loot tier group`/`Combat: Grant` lines. This tells us whether `b5d7190b` was the real inventory cause.
2. **Ranged hit probe (log-only)** in `FortHarvestFlow.cpp` for non-melee abilities, plus a `Combat: Grant` line for the ranged ability when equipped. One run answers "does a gun fire/grant at all and does hit data reach the server".
3. **Damage + ammo for the ranged path via the hook**, using server-side damage from the weapon stat row into `FortAthenaHealth::ApplyDamage`; rebind `ServerReleaseWeaponAbility`; set ammo through the item entry's `LoadedAmmo`.
4. **Expose the high-volume BP natives** that are pure lookups/gives, with real implementations: `GetActorTeam`/`OnSameTeam`/`GetFortPlayerControllerFromActor`, `FortGlobals::IsInAthena`, `FortPawn::IsDead`, `K2_RemoveItemFromPlayer`, `GiveItemToInventoryOwner`, `K2_SpawnPickupInWorld`, `GetFortAbilitySystemComponent`. Consumables mostly unlock from this set.
5. **Death**: drop inventory on elimination, spectate, kill feed, and the alive-count/win path from R3/G9. Needs a test target (bot or second client).
6. **Set pickup root to `CollisionCylinder`** (certain defect from the log), **cut the log spam**, and add the collision audit.
7. Later: building structural support, throwables/traps/vehicles, team end-game, return-to-lobby parity.

## What the user's next single run can confirm

Searching a chest (sound, searched state, items spreading and flying in about 0.4 s); picking up a gun and seeing a quickbar slot; picking up and equipping that gun and pulling the trigger
(expect nothing visible, but we get `Combat:` lines either way); `GC:` lines if a 61 s correction burst repeats. Send `gameserver.log` and `network-rpc.jsonl` from `generated/chrono-server-logs`.

## Update, later on 2026-11-15: what was implemented (all built OK, none client-tested)

| Item | Commit | State |
|---|---|---|
| 49 Blueprint natives declared from behavior snippets (signatures from reflection) | `852108e0` | pushed, untested. The loader should no longer log "Skipping import Function" for them |
| 31 of those rebound to real server implementations in `Private/FortBlueprintNatives.cpp`: team queries (`GetActorTeam`, `OnSameTeam`, `GetTeamAffiliation`), `GetFortPlayerControllerFromActor`, `GetGameStateAthena`, `GetServerWorldTimeSeconds`, item give/remove/quantity/find, `K2_SpawnPickupInWorld(+WithClass)`, `CreateItemEntry`, `GetFortAbilitySystemComponent`, `MakeFortEffectContext`, `BP_FortApplyGameplayEffectToTarget`, building health, `IsInAthena`, `GetNumericCVar` | `852108e0` | pushed, untested. Log `BpNatives: N of M ... rebound` shows what bound |
| Ranged hit damage from target data (`Combat: RangedTargetData`, `Combat: RangedHit`), server-side from the weapon stat row; ammo decremented only when the server AmmoCount is positive | `852108e0` | pushed, untested. First thing to read in the next log: `rowFound=` and `ammo=` on `RangedTargetData` |
| Floor-loot roll logs moved to Verbose | `852108e0` | pushed |
| Elimination drops the victim's inventory as spread pickups; 5 s later the victim's view follows the killer or a survivor | `cd80d9ea` | pushed, untested; spectate is only reached when someone is left alive |
| `ServerCombineInventoryItems` re-created (stack merge), `ServerSpotActor` no-op, `ServerReleaseWeaponAbility` re-created from the retail read (spec input released on the owner's ASC) | next commit | built, untested |

Natives deliberately left at their default result (no safe stand-in yet): `PickLootDrops`, `SpawnProjectile` and the projectile/WaitTarget tasks, `IsCriticalHit`/`IsFatalHit`, `OpenActor`/`CloseActor`, `K2_ConsumeAmmo`, `TossPickup`,
`BP_SpawnLoot`, squad/resurrection queries, and the structural-support natives.

Still not done from the choke-point list: collision audit (needs a run with a known bad spot), skydive movement (needs a failing-run log), structural support/placement rules, throwables/traps/vehicles,
team end-game and return to lobby. Nothing in this section is client-verified.

### Further changes the same day (commits `5dbc71c4`, `9e54f4a8` and the one after; built OK, none client-tested)

* **Likely cause of "guns do not shoot" found and fixed**: `EquipItem` in `FortAthenaInventoryFlow.cpp` never set the spawned weapon's `AmmoCount`, which replicates to the client as the magazine count. Every gun therefore reached the client with an empty magazine. It is now set from the item entry's `LoadedAmmo` (rolled from the weapon's `ClipSize`).
* **Ranged hits no longer change server ammo.** With no server reload, a server-side decrement would be replicated over the client's reloaded value. Reload is client-side only for now, so the inventory's reserve ammo is not consumed (infinite reserve). The retail reload ability (`UFortGameplayAbility_Reload`, native, no cooked Blueprint) is still not ported.
* `PickLootDrops`, `BP_SpawnLoot`, `K2_ConsumeAmmo`, `ServerSetAutoEquipBetterItems` and `ServerReturnToMainMenu` (G9 section 4: sends the engine `ClientReturnToMainMenuWithTextReason`) are implemented.
* **Last-team-standing win check** (duo/squad) replaces the solo-only check; placement in team playlists counts teams left. Solo is unchanged and still requires one player left.
* **Collision audit**: console command `FortCollisionAudit` (or start with `-FortniteCollisionAudit`, runs once 90 s in) logs, per level, scenery static meshes that block pawns but have no physics body on the server. Run it and send the `CollisionAudit:` lines; the output names the levels and example locations.

Genuinely not done, with the reason: skydive movement (no log of a failing run exists; the existing 5 s `MatchFlow: skydiver` lines will show it), structural support and placement rules (retail
building-support code not ported; needs its own work package), projectile/throwable/trap abilities (native `SpawnProjectile` and the WaitTarget/SpawnProjectile tasks have no recovered bodies), server reload,
DBNO/revive, vehicles, and AI/bots. These are separate implementation efforts, each larger than a one-pass fix.
