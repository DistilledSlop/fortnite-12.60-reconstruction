# G8 combat: equip, fire, hit, damage, harvesting and elimination trigger

Research date: 2026-11-13. Research only: no source implementation, staging, build or game run performed.
`binary` means the original 12.60 client, `data` means extracted cooked assets, `source` means current local code;
`inferred` identifies a proposed reconstruction. Nothing here passes G8 without a recorded vanilla-client run.
The 2026-11-13 common brief supplies the runtime baseline; current working-tree code is not a tested binary.

## 1. Findings that change the implementation plan

- **binary:** the damage execution is present: `UFortDamageFormulaExecutionCalculation::Execute_Implementation`
  is **0x141fe2cc0**, primary vtable `0x146055810`, slot 77 / `+0x268`.
  Slot 78 / `0x141feb2b0` returns the description “Deals Damage.”; it is not execution.
- **binary:** weapon virtual `+0x760` is **0x1426a1760** on base/ranged weapons. It assigns the pawn and,
  on authority, sends `ClientGivenTo(Pawn)` through `0x142b43240`. This body does **not** establish a server ability grant.
  The inventory source comment equating that virtual with granting is unsupported.
- **binary:** building damage/weak-spot handling survives, but its resource-award virtual `+0xb28`
  on `ABuildingSMActor` vtable `0x145deede8` resolves to **0x1406801c0**, a no-op.
  The exact server award algorithm cannot be ported from this call. D6's per-hit formula is a design, not evidence.
- **binary:** execution outputs intermediate `FortHealthSet.Damage`, `ShieldDamage` and
  `FortControlResistanceSet.ImpactDamage`; it does not directly subtract pawn Health/Shield.
  Restoring execution without attribute-set callbacks still leaves combat broken.
- **source:** `FortAthenaInventoryFlow.cpp:135` has an opt-in `-FortniteInventoryAbilities` stand-in;
  `FortAthenaHealth.cpp` already grants GAS_AthenaPlayer if loadable and binds a health-depletion listener.
  Extend these paths under the single Native owner; do not introduce duplicate grants/death listeners.

## 2. Equip and ability ownership

**binary:** port in this order: pawn `EquipWeaponDefinition` **0x1423150b0** → reuse/equip helper
**0x142314fe0**, or spawn weapon → initialization **0x1426b34e0** → insert CurrentWeaponList → weapon
`+0x760` **0x1426a1760** → equip helper. The outer function requires authority and reuses a live weapon
whose ItemEntryGuid matches; it also handles the cosmetic harvest-tool definition. Preserve inventory GUID,
WeaponData, WeaponLevel, owner/instigator and current/previous weapon transitions.
`0x1426a1760` calls `+0x440`, assigns pawn through `0x1407247f0`, then sends ClientGivenTo on authority.
Event identity is established by the `ClientGivenTo` FName initializer at `0x1404530f6`/`0x1404530fd`.

**data:** definitions contain soft PrimaryFireAbility and weapon-actor-class references (§6).
Resolve `GetPrimaryFireAbility`, `GetSecondaryFireAbility`, `GetReloadAbility`, `GetOnHitAbility` and
`GetEquippedAbilities`, including inherited defaults; absence of a serialized field is not a null default.
**source:** the current stand-in grants only the first four, once when spawning, with SourceObject=Weapon,
level=ItemEntry.Level and InputID=INDEX_NONE. It has no owned handle ledger/removal/re-equip lifecycle.
**inferred:** reconstruct authoritative granting with per-weapon spec handles and the actual input/activation
contract. Do not invent numeric input IDs or assume INDEX_NONE is correct for every ability.
GiveAbility must run on the authoritative PlayerState ASC after InitAbilityActorInfo(PlayerState, Pawn).
Re-equip must not duplicate specs; inventory removal must cancel/clear the weapon's own grants as required.

**data/binary:** `generated/agents/ctor-defaults/ctor_defaults.json` uses `by_class[Class].defaults`.
AFortWeapon ctor `0x14268d9b0`: bReplicates=true, bRemoveAbilitiesWhenRemovedFromInventory=true,
WeaponLevel=-1, LastFireTime=-10, InputQueueTimePercent=.2, TraceThroughBuildingsLimit=16,
TraceThroughPawnsLimit=16. These are native defaults, not permission to replace item level with -1.
The extraction has no UFortDamageSet/UFortHealthSet entry; recover their attribute constructors and table
initialization rather than filling all multipliers with guessed 1s. Respect named subobject evidence in
`generated/reflection/native_default_subobjects.json`; keep the already-working health/ASC identities.

## 3. Activation, prediction and target data

**source:** UE root below is `/opt/fortnite/UnrealEngine/Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/`.
`Private/AbilitySystemComponent_Abilities.cpp` around 1450, 3103, 3186, 3294 implements activation,
ServerSetReplicatedTargetData, prediction-keyed caching/delegate dispatch and batched RPC handling.
Target data is associated with **ability spec handle + original activation prediction key**; preserve the
current scoped prediction key too. `Private/Abilities/Tasks/AbilityTask_WaitTargetData.cpp:173–246`
demonstrates callback registration, early-cache replay, consume and target-actor validation. This engine
task is an implementation reference, not proof Fortnite uses that task class.

**data:** pickaxe ability GAB_Melee_ImpactCombo_Athena inherits GAB_Melee_GenericCombo; that package
imports FortAbilityTask_PlayMontageWaitTarget, PlayMontageInfoAndWaitForTrigger and
GE_Melee_GenericDamageMakeWeakspot. Parent GAB_Melee_Generic_EventGraph imports the same task,
GameplayAbilityTargetDataHandle and BP_ApplyGameplayEffectToTarget. Imports establish dependencies,
not Blueprint execution order. Recover the trigger graph as part of C3; do not substitute a fire timer.

**binary:** actual montage task ready callback exec **0x142789790** → body **0x1416324a0**:
check Fort ASC/ability → prediction scope `0x140738470` → conditional send via `0x141639830` and
`0x1406fc3c0` (ServerSetReplicatedTargetData, spec/original/current keys, data, application tag), otherwise
a conditional local-authority route `0x14160c4c0` → broadcast Triggered `0x14161f520` → `0x140701840`
→ close scope. It does not unconditionally end the task. Generated PlayMontageWaitTarget currently has
delegates/montage fields but lacks this factory, callbacks and lifecycle. Restore them through generator inputs.
The separate WaitTargetSelection callback **0x141632740** (exec `0x1427898f0`) sends through the same
RPC path when non-authority/local and ability byte +0x1a8 is zero (`0x1416398a0`); do not mistake it
for the proven pickaxe task or name that byte's enum without checking its original layout.

**binary:** Fortnite container application exec **0x1428f61d0** → virtual `+0x480` **0x1415ea920**
→ virtual `+0x478` **0x1415eab90** with activation info, target data and container tag. Effect lists come
from `0x1415fb3d0`/`0x1415fb4d0`; target effects call **0x1406b8100**, the engine GameplayAbility
ApplyGameplayEffectToTarget path (`Private/Abilities/GameplayAbility.cpp:1696`), collecting effect handles;
the other list uses `0x1406b7be0`. Target hit results are also passed to weapon `+0x680` **0x1426ae530**.
`UFortAbilitySystemComponent::BP_FortApplyGameplayEffectToTarget` exec `0x1427807f0` → helper
`0x1415ec3f0` → base `0x1406b7f10` is another application route. Do not assume every weapon uses containers.

**data/binary:** rifle GA_Ranged_GenericDamage imports FortGameplayAbility_RangedWeapon and its damage GE.
Ranged activation **0x14161b540** (slot 124; vtable `0x145d23870`, ctor `0x141615810`) requires an instanced
native ability bound to a ranged weapon and nonempty default/tagged effect containers; it rejects BP activation,
binds ASC target-data delegate `0x1406f2ec0` (handle retained +0xa60), then calls **0x14163c910**.
Registered ranged callback **0x141632980** (slot 154) checks spec/object, opens prediction scope, sends target
data via `0x1406fc3c0` when locally controlled/nonauthority, commits through `+0x308`, selects tagged container
with `0x1415facd0` and dispatches `+0x478` → `0x1415eab90` → `0x1406b8100`. It cleans keyed target data
through `0x140701840`. This is a real native receiver-to-GE path, with ~588 lines of weapon/hit branches to lift.
EndAbility **0x141623c00** (slot 128) unregisters delegates/cleans data and calls base `0x1415f3100`;
it is not the fire entry. Preserve ammo/cadence/reload, deduplication and prediction; never accept client damage amounts.
**inferred:** validate reported actor, range, line of sight, weapon ownership/state and repeat hits at the
server boundary, while retaining original targeting/filter code wherever present. The melee server receiver and
Blueprint trigger-to-application edges still require C3's targeted lift; imports alone do not prove them.

## 4. Damage execution and health/death boundary

**binary:** decompiled Execute `0x141fe2cc0` is ~1,544 lines including diagnostics. Constructor
**0x141fdd3a0** registers 20 captures. Static capture definitions: accessor **0x141fe12e0** → initializer
**0x141fdbf80**, data `0x14813a120`, stride 0x40. Capture Source=0/Target=1 plus snapshot byte are
explicit (`0x100` means source+snapshot; `1` target+live). Port the CDO registrations before executing specs.

| Definition offset | Attribute read | Capture |
|---|---|---|
| 0x000 | DamageSet.OutgoingBaseAbilityDamageMultiplier | source snapshot |
| 0x040 | DamageSet.OutgoingBaseDamage | source snapshot |
| 0x080 | DamageSet.OutgoingBaseEnvironmentalDamage | source snapshot |
| 0x0c0 | DamageSet.OutgoingAbilityDamage | source snapshot |
| 0x100 | DamageSet.OutgoingBaseImpactDamage | source snapshot |
| 0x140 | DamageSet.ZoneCritMultiplier | source snapshot |
| 0x180 | DamageSet.DiceCritChance | source snapshot |
| 0x1c0 | DamageSet.DiceCritMultiplier | source snapshot |
| 0x200 | DamageSet.KnockbackMagnitude | source snapshot |
| 0x240 | DamageSet.KnockbackZAngle | source snapshot |
| 0x280 | DamageSet.ShieldDamageScale | source snapshot |
| 0x2c0 | DamageSet.HealthDamageScale | source snapshot |
| 0x300 | DamageSet.StunTime | source snapshot |
| 0x340 | HealthSet.DamageResistance | target live |
| 0x380 | HealthSet.DamageVulnerability | target live |
| 0x3c0 | ControlResistanceSet.ImpactDamageResistance | target live |
| 0x400 | ControlResistanceSet.ImpactDamageMultiplier | target live |
| 0x440 | HealthSet.CurrentShield | target live |
| 0x480 | DamageSet.CritRating | source snapshot |
| 0x4c0 | HealthSet.Armor | target live |

Getter identities additionally checked: `0x141feaa10` BaseDamage, `0x141feaac0` EnvironmentalDamage, `0x141fea960` AbilityDamage, `0x141fe7dc0` DiceCritChance, `0x141fe7e70` DiceCritMultiplier, `0x141fe7ac0` DamageResistance, `0x141fe7b70` DamageVulnerability.

**binary:** execution requires a Fortnite effect context (`0x1415f4460`) and target ASC; otherwise it exits.
It selects base/environmental capture according to target, obtains source damage-provider interface
through **0x1415fa0b0**, resolves source/hit location, distance, surface, charge, level and damage zone.
Provider virtuals `+0x10/+0x18/+0x20/+0x28/+0x30/+0x38/+0x40` supply damage/impact/surface/charge/
crit/zone inputs; map their concrete weapon implementations before converting calls to typed C++.
It evaluates captured aggregators with source/target tags and scoped modifiers, not just raw current values.
Preserve the per-context cached crit roll (context key at +0xb0, execution object +0x40/+0x44), the
zone/crit restrictions, and curve-based CritRating and Armor conversions from globals (`+0x2c0/+0x2d0`).
The damage branch applies spec stack count, vulnerability and resistance (resistance upper cap .95).
Impact resistance clamps to [0,1]; impact has its own multiplier. These are distinct from hit point damage.

**binary:** shield branch excludes native tags at gameplay-tag singleton offsets **0x360, 0x368, 0x3d0**;
resolve their names using existing native tag runtime before choosing bypass rules. With eligible shield,
ShieldDamage=min(CurrentShield, damage*ShieldDamageScale); subtract the absorbed damage divided by
ShieldDamageScale from the unshielded portion, then multiply the remainder by HealthDamageScale.
Zero-scale and excluded-tag branches must follow the original control flow. Outputs are additive meta
attributes **HealthSet.ShieldDamage**, **HealthSet.Damage**, **ControlResistanceSet.ImpactDamage**.
Context flags +0x71/+0x72 carry critical results; +0x73 gates knockback/stun payload at +0x74/+0x78/+0x7c.
Current `Private/FortServerGameplayEffectContext.cpp` serializes these fields but does not supply every
original source-object/provider behavior; casting an arbitrary stock context to the original layout is invalid.

**binary:** HealthSet vtable callbacks: PreGameplayEffectExecute **0x141bc11d0**;
PostGameplayEffectExecute **0x141bbd020**; PreAttributeChange **0x141bbfa90**.
Actual player set is RegenHealthSet: pre **0x141bc15d0**, post **0x141bbebf0**; preserve parent behavior.
Post consumes/resets ShieldDamage and subtracts CurrentShield; consumes/resets Damage and subtracts Health,
clamps, emits damage/gameplay events, and handles healing/max changes separately. At depleted health it
consults a delegate at set +0x348 (DBNO/interception boundary), then broadcasts the out-of-health delegate
at **+0x358** when health remains <=0, the prior-dead guard permits it, and damage is non-negligible.
The ordinary damage delegate is +0x2e8. Follow the decompile's old-health/DBNO ordering.

**source/inferred:** current health-change lambda already calls HandlePlayerEliminated at <=0 and
!bIsDying; direct ApplyDamage also calls it. Use one idempotent authoritative elimination entry and preserve
instigator, source, hit and damage tags. The lambda is bound once per ASC but captures the first Pawn:
refresh/unbind across bus/respawn avatar replacement. Do not infer storm from missing context for weapon kills;
CapturedTargetTags are not automatically the damage-source/effect tags. Do not invoke elimination during
an intermediate zero-health callback before original DBNO processing finishes.
AFortPawn::OnDeathServer is a Blueprint event, not a recovered native death implementation. D6/R3 document
the missing player match-elimination caller: delegate/event depletion → reconstructed elimination entry →
death report/DeathInfo/drops → RemoveFromAlivePlayers **0x14195e620**. Do not claim that final caller is ported.

## 5. Building hits, destruction and harvesting

**binary:** SM damage callback **0x141c0b650** first calls building base **0x141baaf60**, obtains context,
controller, remaining health and effective damage clamped against overkill. It tests virtual `+0xe60`,
matches hit weak-spot data to the controller/building entry (**0x142388080**), clears/advances the spot
and chain, records WeakPointHit and clears weak-spot credit when no valid entry exists; this does not reject
the whole damage hit. It checks near-zero damage/max-health fraction, calls `+0xe68`, and for an eligible player
invokes **+0xb28(pawn, damage, weakspot)**.
That last target is stripped (§1); do not use ClientReportDamagedResourceBuilding as a server grant implementation.
Living damaged buildings can invoke **0x1423b5c20(controller, building)** to create/update weak spots.
That routine requires authority/world, obtains view direction, performs building `+0xab8` placement using a
normalized **400-unit** view ray, stores hit location/normal and schedules through timer manager `0x1446278d0`
using GetWeakSpotEnabledTimeout **0x142753040** plus unresolved global `_DAT_1478c32a8`; it broadcasts updates.
Preserve that adjustment, placement failure, expiry, per-controller identity and chain behavior; never trust a client critical flag.

**binary:** building HandleOutOfHealth **0x141bac110** guards bDestroyed, updates feedback/navigation,
flushes dormancy where needed, sets bDestroyed, schedules LifespanAfterDeath (selected tags use .1s),
calls virtual `+0x930`, calls BP OnDeathServer via **0x142734dc0**, then broadcasts OnDied.
Do not substitute plain Destroy() or route only through AActor::TakeDamage; wire the GAS damage/death callbacks.
**binary:** ctor defaults: ABuildingActor MaxHealthInitializationValue=-1, SavedHealthPct=1,
bAllowTeamDamage=true; ABuildingSMActor ResourceType=5, MaxResourcesToSpawn=-1. These are sentinels/defaults,
not evidence that every placed prop yields wood or that default max health is one.

**data:** Balance/AthenaResourceRates x=0 ResourceLow/Small/Medium/High/Super = **10/11/19/33/72**;
Apollo playlist counterpart = **9/9.9/17.1/24.75/54**. For these five rows x=1=.15, with linear interpolation
and constant extrapolation; other rows differ (Balance PropWoodLow/PropStoneLow/PropMetalLow x=1=.32).
Trace the active playlist/table and each building's BuildingResourceAmountOverride before evaluating.
**inferred:** the missing grant must reconstruct row selection, curve input, damage/max-health scaling,
weak-spot interaction, rounding, cumulative caps, final-hit and inventory-overflow behavior; these values
do not establish “10 per hit” or an extra resource multiplier of two. Avoid doubling weak-spot damage twice.

**binary:** client receipt **0x1423727b0**, ClientReportDamagedResourceBuilding(Building, ResourceType,
Amount, bDestroyed, bJustHitWeakspot), resolves a resource item/count and broadcasts feedback listeners;
it is presentation, not inventory authority. **inferred:** grant through FortAthenaInventory::GiveItem with
the real resource definition/count/cap, update replicated entries, then send receipt with the accepted amount.
The resource-award recreation remains a parity gap until its rules are independently justified and exercised.

## 6. Assets and safe staging

All paths below are `/Game/` packages; append `.basename` for objects, `.basename_C` for Blueprint classes.
Use original assets, including parent classes/effects; do not create substitute zero-damage GEs.

| Root or chain (data) | Required purpose |
|---|---|
| Abilities/Player/Generic/Traits/DefaultPlayer/GAS_AthenaPlayer | 31 default abilities, 2 granted effects |
| Athena/Items/Weapons/WID_Harvest_Pickaxe_Athena_C_T01 | PrimaryFire → Athena/Items/Weapons/Abilities/GAB_Melee_ImpactCombo_Athena |
| Weapons/FORT_Melee/Blueprints/B_Athena_Pickaxe_Generic | Pickaxe actor soft class |
| Abilities/Weapons/Melee/GAB_Melee_GenericCombo → GAB_Melee_Generic_EventGraph | Montage/targeting parent behavior |
| Abilities/Weapons/Melee/GE_Melee_GenericDamageMakeWeakspot → GE_Melee_GenericDamage | GenerateWeakSpots tag + damage execution |
| Athena/Items/Weapons/WID_Assault_Auto_Athena_C_Ore_T02 | PrimaryFire → Abilities/Weapons/Ranged/GA_Ranged_GenericDamage |
| Weapons/FORT_Rifles/Blueprints/Assault/B_Assault_Auto_Athena | Rifle actor soft class |
| Abilities/Weapons/Ranged/GE_Ranged_GenericDamage → GameplayEffectTemplates/GET_DirectPhysicalDamage | Physical damage; GA also imports GE_Impulse |
| GameplayEffectTemplates/Get_DirectDamageParent → GET_DamageParent | GET_DamageParent serializes FortDamageFormulaExecutionCalculation |
| Athena/Balance/DataTables/AthenaMeleeWeapons / AthenaRangedWeapons | Pickaxe row Harvest_Pickaxe_Athena_C_T01; rifle row Assault_Auto_Athena_C_Ore_T03 despite T02 WID |
| Athena/Balance/DataTables/AthenaResourceRates; Athena/Playlists/Apollo/AthenaResourceRates | Candidate harvest curves; determine active table |

Concrete fixture CDOs (presence is not placement/runtime proof; health is inherited, not serialized):
- Wood: `Building/ActorBlueprints/Prop/Arid_Desert_DeadTree_01`, override `Balance/DataTables/ResourceRates:ResourceWoodMedium`.
- Stone: `Athena/Apollo/Environments/BuildingActors/Rocks/Prop_Rock_01`, override `Athena/Balance/DataTables/AthenaResourceRates:ResourceStoneLow` (x=0:12, x=1:.15).
- Metal: `Athena/Apollo/Environments/BuildingActors/OilRig/Props/Apollo_OilRig_BarrelRack_01`, same Athena table, `PropMetalMedium` (x=0:5, x=1:.15).
Stage these three packages and the separate `Balance/DataTables/ResourceRates` root explicitly for fixtures.
Resource definitions exist at `Items/ResourcePickups/{WoodItemData,StoneItemData,MetalItemData}` and Athena-prefixed
equivalents; MaxStackSize=999. Current inventory uses base WoodItemData. Resolve native helper `0x141efa440`
→ `0x141e63190` before selecting the family; do not silently substitute Athena definitions.

GE_Melee_GenericDamage also serializes an additive scoped source-snapshot modifier on
OutgoingBaseAbilityDamageMultiplier, scalable value 0; it contains no active set-by-caller damage key.
Ranged inherits the execution through GET_DamageParent → native GameplayAbilities.GameplayEffect;
no execution field on a child is not evidence that ranged damage lacks execution. GET_DirectPhysicalDamage
uses GameplayCue.Damage.Physical on FortHealthSet.Damage and damage/instant/physical tags.

GAS_AthenaPlayer's complete ability membership (brace groups denote separate packages; Blueprint suffix `_C`):

- Native `/Script/FortniteGame.FortGameplayAbility_{Jump,Sprint}`.
- `/Game/Abilities/NPC/Generic/{GA_SlamDoorOpen_Instigator,GA_SlamDoorOpen_Victim,GAB_AthenaDBNO,GAB_AthenaDBNORevive,GAB_CarryPlayer,GAB_DropPlayer,GAB_ThrowPlayer}`.
- `/Game/Abilities/Player/Generic/Traits/DefaultPlayer/{GA_DefaultPlayer_Death,GA_DefaultPlayer_InteractSearch,GA_DefaultPlayer_InteractUse,GA_DefaultPlayer_PetOtherPet}`.
- `/Game/Abilities/Player/Interrogation/{GAB_InterrogatePlayer_Reveal,GAB_InterrogatePlayer_Search}`.
- `/Game/Athena/DrivableVehicles/{GA_AthenaEnterVehicle,GA_AthenaExitVehicle,GA_AthenaInVehicle,GA_Vehicle_ExitHoldEvent}`.
- `/Game/Athena/Environments/Blueprints/DudeBro/GA_DudeBro_Vent`; `/Game/Athena/Environments/Blueprints/SurfaceEffects/GAB_SurfaceChange`.
- `/Game/Athena/Items/Consumables/Grenade/GA_Athena_Grenade_Rethrow`; `.../PurpleStuff/GA_Athena_Slurp_OLD`;
  `.../ThrownConsumables/IceGrenade/GA_Athena_RemoveIceFeetOnWater`; `.../ZipLine/GA_Athena_ZipLine_Smash` (`...` is the same Consumables prefix).
- `/Game/Athena/Items/EnvironmentalItems/HidingProps/{GA_Athena_HidingProp_Hide,GA_Athena_HidingProp_JumpOut,GA_Athena_HidingProp_LandedOn}`;
  `/Game/Athena/Items/EnvironmentalItems/SCMachine/GA_Athena_SCMachine_Passive`.
- `/Game/Athena/Items/ForagedItems/Rift/GA_Rift_Athena_Skydive`; `/Game/Creative/Abilities/Siphon/GA_Creative_OnKillSiphon`.

Effects: `/Game/Athena/Playlists/DefaultModifiers/{GE_DefaultResourceCap,GE_DefaultTurboBuildSettings}`.
Both serialized levels are **0**; current health code substitutes 1. Establish original application-level semantics
before changing this behavior. Grant only after load succeeds; current code marks the PlayerState granted before
checking load, preventing retry after a failed load. Grant once per owner, initialize each new pawn's actor info.

Future implementer commands (not executed here except the narrower dry run described below):
```sh
python3 -I porting/tools/stage_server_content.py \
 /Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_AthenaPlayer \
 /Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Athena_C_T01 \
 /Game/Weapons/FORT_Melee/Blueprints/B_Athena_Pickaxe_Generic \
 /Game/Athena/Items/Weapons/Abilities/GAB_Melee_ImpactCombo_Athena \
 /Game/Athena/Items/Weapons/WID_Assault_Auto_Athena_C_Ore_T02 \
 /Game/Weapons/FORT_Rifles/Blueprints/Assault/B_Assault_Auto_Athena \
 /Game/Abilities/Weapons/Ranged/GA_Ranged_GenericDamage \
 /Game/Athena/Balance/DataTables/AthenaResourceRates \
 /Game/Athena/Playlists/Apollo/AthenaResourceRates --dry-run --max 20000
```
Review, then repeat without `--dry-run`. Default exclusions skip materials/textures/particles/Niagara/audio/
fonts/widgets and their traversal. **Do not pass --include-materials** for this combat batch: original shader-bearing
packages caused the earlier server material crash. Explicit roots bypass exclusions; keep them gameplay-only.
Explicit soft ability/actor roots above are necessary; `--soft` broadly expands unrelated dependencies.
Existing stage files are never removed/overwritten; exclusions do not cleanse a previously contaminated stage.
Missing `/Script` modules and engine imports require code/compatibility work, not more `/Game` copying.
Research dry run with GAS, melee ability, ranged ability and both resource tables: 2,208 visited, 1,686 accepted,
522 excluded, 0 missing, 661 would-copy files / 8.89 MB, 2,835 existing. This proves dependency inventory only.

## 7. Ordered work packages (one Native/build/stage owner)

All new handwritten files go in `porting/native/Source/FortniteGame/Private/`; generator changes go in
`porting/behavior/g8_combat_runtime/` and `porting/generator_policies/g8_combat_runtime/`, then regenerate.
Never hand-edit Generated/. Keep current edits; coordinate integration points with their current owners.
The diagnostic fixture/log prefixes below are **proposed implementation**, not existing runnable features.

**G8-C1 — content and prerequisites.** Own new `FortAthenaCombatContent.cpp/.h`, narrow SetupPlayer integration
in FortAthenaHealth.cpp, and staging under the shared owner. Assets: all §6 roots/hard closures, actual rifle
stats, resource definitions and original test building. Restore ctor/CDO defaults, subobjects and building
ASC/HealthSet ownership/damage/death bindings. Check effective GE execution and native policies after load;
fix grant-on-success/retry and level handling.
Test T0 (§8); require `Combat: ContentReady abilities=31 effects=2` plus named resolved actor/ability/GE classes,
execution class/captures and no missing native functions/CDOs. Done: real loadable definitions, not file presence.

**G8-C2 — equip/grants.** Own new `FortAthenaCombatEquip.cpp/.h` and narrow inventory equip calls.
Port §2 addresses in order, resolve definition accessors including EquippedAbilities, reconstruct missing grant
ledger with cleanup and source object. Assets: both WIDs, actor BPs and resolved abilities. Test T0/T1: log
`Combat: Grant` with owner/spec/source/input policy, equip away/back, remove/regrant. Done: no duplicate/stale
specs, current weapon usable by client, grants survive normal possession transition without leaking old avatar.

**G8-C3 — fire/target-data execution.** Own generator inputs for ranged ability, PlayMontageWaitTarget and
needed weapon virtuals; new `FortAthenaCombatTargeting.cpp/.h` for explicitly missing server glue.
Lift montage factory/lifecycle and ready callback 0x1416324a0, its receiver and validation; ranged activation
0x14161b540 → 0x14163c910/0x141632980 → 0x1415eab90, cleanup 0x141623c00; restore actual melee BP apply edge.
Assets: original melee/ranged abilities, montage/target specs, GEs. Preserve prediction keys, early-data replay,
consume, cancellation/end cleanup, server hit acceptance, ammo/cadence/reload. Test T1/T2: activation → one
`Combat: TargetDataAccepted` → `Combat: EffectApplied` per accepted hit, with same spec/key/hit identity.
Done: actual mouse-triggered swing/fire traverses original ability path; denial/cancel/repeat does not apply damage.

**G8-C4 — execution and meta attributes.** Own generator policy/constructor override for formula and attribute
sets, new `FortAthenaCombatDamage.cpp/.h`; integrate current Fortnite context provider rather than aliasing layout.
Port 0x141fdbf80 → 0x141fdd3a0 → source provider dependencies → 0x141fe2cc0 → HealthSet callbacks
0x141bc11d0/0x141bbd020/0x141bbfa90 and RegenHealthSet overrides 0x141bc15d0/0x141bbebf0.
Assets: GE hierarchy, weapon stats, actual attribute/armor/crit curves. Test T1/T2 including shield/no-shield,
head/ordinary hit, building/player, storm bypass and meta reset; log `Combat: DamageResolved` and before/after
attributes. Done: no capture warnings, correct context/tags/scales, meta values reset, HUD agrees with server.
Focused captured-input checks are warranted for scaling/crit/prediction; no duplicate per-accessor probe suite.

**G8-C5 — buildings and weak spots.** Own new `FortAthenaCombatBuilding.cpp/.h` plus generator bindings.
Lift base 0x141baaf60 → SM 0x141c0b650 → weakspot lookup 0x142388080/update 0x1423b5c20 →
out-of-health 0x141bac110; preserve Blueprint hooks, dormancy, lifespan and one-shot OnDied.
Assets: §6 resource props, wall, weakspot data/class, inherited health/stat defaults. Test T1: ordinary hit,
validated weakspot and destruction; focused stale/false-credit check; `Combat: BuildingHit`, `Combat: WeakSpot`,
`Combat: BuildingDestroyed` plus actual client weakspot/destroy screenshots. Done: real replicated prop damage
and destruction; forged/expired marker rejected; non-harvest rifle damage works without awarding materials.

**G8-C6 — resource award recreation.** Own new `FortAthenaCombatHarvest.cpp/.h`; call existing inventory API.
At stripped +0xb28 boundary implement justified active-table/row/curve/rounding/cap/overflow rules; label
unproven rules as stand-ins and keep parity work open. Use real damage/weakspot result, resource definitions,
all applicable §6 tables and original ClientReportDamagedResourceBuilding RPC contract 0x1423727b0.
Test T1 with normal/weakspot/final hit, all three materials and cap/overflow; log `Combat: ResourceAward`
row/table/input/raw/rounded/accepted/before/after and receipt amount. Done: authoritative inventory and client
feedback agree, each accepted hit grants once; exact original yield parity requires evidence beyond these curves.

**G8-C7 — depletion to elimination.** Own narrow FortAthenaHealth.cpp/FortAthenaEndGame.cpp integration,
coordinate existing death work. Wire final original depletion boundary (§4), retain DBNO choice and one-shot
death guard, refresh pawn on respawn; call existing elimination path then R3 RemoveFromAlivePlayers.
Assets: GA_DefaultPlayer_Death, applicable DBNO effects/abilities and damage tags. Test T2: player kill and
storm death separately; `Combat: Eliminated` once with correct cause/instigator, death report and PlayersLeft.
Done: victim death UI and kill attribution, inventory drops and alive count agree; G9 victory/results remain separate.

**G8-C8 — coherent integration evidence.** Own new `FortAthenaCombatProbe.cpp/.h` only for deterministic test
setup plus reviewed hook; all §6/test assets. Add fixtures specified below and bounded structured combat logs.
After C1–C7 review, Native owner schedules build/stage and vanilla run at the agreed checkpoint, retaining IDs.
Test T0/T1/T2. Done: screenshots/logs show real input → hit → resource
or health/death; no direct test damage substitutes for ability traffic. Mark untested work build/runtime pending.

## 8. Exact future tests and evidence

Implement proposed `-FortniteCombatProbe=content|harvest|rifle|duel` in C8; this is diagnostic scene setup only.
After ordinary landing/possession: keep player still, set control rotation yaw/pitch=0 through the owning client,
place known original prop collision 100 units ahead for harvest, wall 1,000 ahead for rifle; give/equip item through
normal inventory API. Ready must mean replicated fixture/weapon/ASC ready, not just spawned. Duel waits for two
clients, puts A/B 1,000 units apart facing each other, B stationary, A rifle equipped. Log fixture class/health/row,
source IDs and `CombatProbe: Ready`; separate phase `CombatProbe: RetestReady` enables weakspot/cap/re-equip cases.
Align fixture collision to the player's eye-height ray, checking bounds/trace; distance from pawn origin alone is insufficient.
Fixture must not synthesize target-data, damage, resource awards or death. Screenshot aim first if collision misses.
Proposed `-FortniteCombatCase=wood|stone|metal|weakspot|cap|overflow|final` selects setup and logs its inputs.
Weakspot case rotates owning-client aim to the actual server-selected spot before RetestReady; log spot/aim/hit. Cap/overflow cases preseed the selected material to 999/998 through normal inventory API;
compare accepted delta with available capacity and justified computed award. Final case repeats genuine strikes until destruction; compare per-hit/final receipts and inventory. Run material cases against each declared prop.
Expired/forged-credit checks use focused validation inputs; they do not prove vanilla-client behavior or pass G8.

Common shell prefix for future owner (run duration includes frontend login):
```sh
FN_RENDERER=dxvk FN_LP_NUM_THREADS=4 FN_SERVER_MAP=/Game/Athena/Apollo/Maps/Apollo_Terrain \
FN_SERVER_URL_OPTS='?RequiredPlayers=1' \
FN_SERVER_ARGS='-FortniteSkipUnconvertedLegacyShaderMaps -FortniteTolerateNeedLoadDependencies -FortniteSkipEngineClassNetChecksum -FortniteWarmupSeconds=20 -FortniteInventoryAbilities -FortniteCombatProbe=harvest' \
FN_RUN_LABEL=g8-harvest \
FN_ACTIONS='0:wait,UI.State.Startup.SubgameSelect,900 20:click,640,592 0:wait,ShouldShowBeginningOfSeasonDialog,600 25:click,1144,473 0:wait,CombatProbe:.Ready,900 3:shot 4:mhold,3 8:shot' \
porting/tools/vanilla_play_run.sh 1800
```
T0: use probe `content`, final actions `0:wait,CombatProbe:.Ready,900 3:shot`; review ContentReady and grants.
The action driver's wait can time out and continue: require Ready/RetestReady before attack; abort/fail if absent.
T1: harvest command above; run each declared case after its setup/aim and Ready/RetestReady handshake;
capture ordinary + weakspot + final hit and all three resource types. For `rifle` wall fixture use
`0:wait,CombatProbe:.Ready,900 3:shot 4:mhold,1 6:key,r 10:mhold,1 12:shot`; verify ammo/reload, damage once
per actual accepted shot and no materials. Replace the ordinary tree with each fixture's declared resource class.
T2: use `?RequiredPlayers=2`, probe `duel`; prepare second prefix before first client per tools/README.md, then
start `porting/tools/second_client.sh <run> --username LocalPlayer2 --renderer dxvk --actions` with the same
frontend clicks and `0:wait,CombatProbe:.Ready,900 3:shot 20:shot` (B never fires). First client final actions:
`0:wait,CombatProbe:.Ready,900 3:shot 4:mhold,8 14:shot`; prove shield spillover and lethal transition, then run
separate storm fixture. Second-client helpers are instructions to the run owner, not evidence this research ran them.

Evidence under `porting/generated/runs/<date>-g8-*/`: server.log, client.log, shots/, meta.json and client2/ logs/shots.
Build logs under generated/builds/. Failure means no gate claim: missing activation/data/effect, native stub/missing
function, capture warning, NetChecksumMismatch, RPC mismatch, fatal/GC errors or unexplained duplicated hit.
Screenshots must show wood/stone/metal count change, real weakspot/destruction, ammo/health/shield and victim death;
Server-only success logs are insufficient; do not rerun unrelated frontend suites for each helper.

WP: research-combat. Result: done (research plan; implementation pending).
What I changed: this file only. What I observed: binary/data/source evidence cited above; no game runs.
Gate impact: none; G8 unchanged.
Follow-ups: C1–C8, especially stripped resource grant, authoritative grants, melee receiver/graph and death caller.

## 9. Harvest award rule (re-creation; the client's award virtual `+0xb28` is the no-op `0x1406801c0`)

Evidence class: **public behaviour + cooked data**, not recovered code (added 2026-11-13).

- Observed in play (community guides, 2018–2020): each harvestable object yields a **fixed total** when broken
  (examples: trees about 26 wood when broken in 3 hits with both weak spots hit; fences 17 wood); weak-spot hits do
  **double damage**, so the object breaks faster and each swing yields more; critical pickaxe hits yield more.
  Sources: <https://fortnite-archive.fandom.com/wiki/Harvesting_Tool_(Battle_Royale)>,
  <https://mein-mmo.de/en/fortnite-hit-weak-points-while-mining-what-is-it,418221/>,
  <https://thegamehaus.com/fortnite/fortnite-analysis-how-to-farm-efficiently/2019/01/07/>.
- Cooked data supplies every input: the building's resource type (`ResourceType`), its total
  (`BuildingResourceAmountOverride` curve row, evaluated through `AthenaResourceRates`), its max health, and the
  weak-spot multiplier from the pickaxe's hit (D6 §3 lists the tables).
- Rule to implement (stand-in, mark it so): `award = round(Total * DamageDealt / MaxHealth)` per hit, with the remainder
  so that the sum over all hits equals `Total` when the building is destroyed; add to the inventory stack (respecting the
  stack max) and call `ClientReportDamagedResourceBuilding(Building, ResourceType, award, bDestroyed, bJustHitWeakspot)`.
- Validate against the public numbers above with the vanilla client: a full-health tree broken with weak-spot hits must
  give its table total; the HUD wood counter must match the server inventory after each hit.
