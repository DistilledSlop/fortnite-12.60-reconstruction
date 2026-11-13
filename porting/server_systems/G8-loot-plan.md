# G8 loot implementation plan — 12.60, 2026-11-13

Research only. No source changes, staging, builds or game runs were performed.
This plan does not move G8. Existing working-tree edits belong to their owners.
`source` means inspected local source; `binary` means the supplied runtime image;
`data` means offline inspection of extracted cooked content; `inferred` marks a
proposed server reconstruction. Paths are relative to porting/; Private/ means
native/Source/FortniteGame/Private/, unless an explicit path is supplied.

## Findings that change the implementation plan

- **binary:** the exposed client loot rolling entry points have no rolling body.
  `UFortKismetLibrary::PickLootDrops` exec `0x142975a00` and
  `PickLootDropsWithNamedWeights` exec `0x142975cd0` call the shared false-return
  stub. `K2_SpawnPickupInWorldWithLootTier` exec `0x142974550` calls
  `0x140738030`, which returns an empty array. Their signatures retain
  WorldContextObject, output entries, TierGroupName, WorldLevel, ForcedLootTier;
  the named version additionally takes `TMap<FName,float>`.
- **binary:** correct `ABuildingContainer::SpawnLoot` is `0x141bec430`.
  Constructor `0x141bccf20` installs primary vtable `0x145f10cc8`; slot `+0xee0`
  points there. BP_SpawnLoot exec `0x14273c1f0` calls `0x141bd5720`, which calls
  that slot (third argument 4, fourth/fifth zero). SpawnLoot logs, checks role,
  optionally gets the world, and unconditionally returns false at `0x141bec504`.
  The previously cited `0x141bec150` is a different function, not a roll algorithm.
- **source:** D4's four-step rolling description is explicitly a standard-scheme
  inference. Exact fractional counts, category scheduling, named multipliers,
  quota/streak-breakers, tags, recursion and bonus semantics cannot be claimed
  recovered from these stubs. Table values constrain them but do not prove them.
- **source:** `Private/FortAthenaLoot.cpp` currently spawns a resolved pickup and
  binds only ServerHandlePickup. Its immediate transfer, 300-unit range and lack
  of flight/slots/swap are labeled stand-ins. It rejects partial/multi pickups.
  `FortniteAthenaGameplay.cpp::StartPlay` does not currently install this module.
  Do not describe this source as verified loot gameplay.

## Exact row layouts and generator entry point

**data:** `generated/reflection/struct_blocks.json` is pinned input. Both rows
inherit `FTableRowBase`, align to 8, have native construction/destruction/copy,
and have no native custom serializer. Preserve every field, including apparently
unused fields; a reduced handwritten row type is not compatible recovery.

| FFortLootTierData, size 224; construct identity `0x14298ae50` | Offset | C++ type |
|---|---:|---|
| TierGroup | 8 | FName |
| Weight | 16 | float |
| QuotaLevel | 20 | ELootQuotaLevel::Type (byte enum) |
| LootTier / MinWorldLevel / MaxWorldLevel | 24 / 28 / 32 | int32 each |
| StreakBreakerCurrency | 40 | FString |
| StreakBreakerPointsMin / Max / Spend | 56 / 60 / 64 | int32 each |
| LootPackage / LootPreviewPackage | 68 / 76 | FName each |
| NumLootPackageDrops | 84 | float |
| LootPackageCategoryWeightArray / MinArray / MaxArray | 88 / 104 / 120 | TArray<int32> each |
| GameplayTags / RequiredGameplayTags | 136 / 168 | FGameplayTagContainer each |
| bAllowBonusLootDrops | 200 | native bool byte |
| Annotation | 208 | FString |

| FFortLootPackageData, size 200; construct identity `0x14298adf0` | Offset | C++ type |
|---|---:|---|
| LootPackageID | 8 | FName |
| Weight | 16 | float |
| NamedWeightMult | 20 | FName |
| PotentialNamedWeights | 32 | TArray<FName> |
| Count / LootPackageCategory | 48 / 52 | int32 each |
| GameplayTags | 56 | FGameplayTagContainer |
| RequiredTag | 88 | FName |
| LootPackageCall | 96 | FString (not FName) |
| ItemDefinition | 112 | TSoftObjectPtr<UFortItemDefinition> |
| PersistentLevel | 152 | FString |
| MinWorldLevel / MaxWorldLevel | 168 / 172 | int32 each |
| bAllowBonusDrops | 176 | native bool byte |
| Annotation | 184 | FString |

**binary:** the pinned records leave bool offsets null and label them bitfields.
The auxiliary `generated/reflection/struct_bool_setters.json` resolves package
setter `0x140ea2690` to `mov byte [rcx+0xb0],1`, tier setter `0x14076c610` to
`mov byte [rcx+0xc8],1`. Emit native byte bools, not an invented packed mask.
Do not change the pinned struct records to repair this.

**binary:** ops vtables `0x14642d468` / `0x14642da40`, Construct slot `+0x18`,
reach package constructor `0x142988f10` / tier constructor `0x142023d90`.
Package defaults: Weight=0, Count=0, category=-1, Min/MaxWorldLevel=-1,
bonus=false, empty names/strings/containers and null soft reference.
Tier defaults: Weight=0, QuotaLevel=0, LootTier=0, Min/MaxWorldLevel=-1,
streak points=0, NumLootPackageDrops=0, bonus=false, empty names/strings/arrays/tags.
Recover these constructors into generation policy: cooked properties can omit
defaults. Class-only `ctor_defaults.json` does not supply these struct defaults.

**source:** `gen_native_classes.py:4867` builds roots from emitted signatures and
accepted bodies. The rows are absent because nothing roots them.
`TypeGate.build_struct` (~5700) resolves parent/members; `emit_types` (~6233)
closes dependencies; `render_struct_header` (~6032) writes headers.
Add `generator_policies/athena_loot_runtime/source_policy.py` plus
`struct-contracts.json`, patterned on `native_checkpoint3058_runtime`:
pin construct identity/package/size/alignment/property hash; `add_roots` both
rows; wire the import and add_roots call beside existing explicit-root policies.
Add narrowly scoped model/render hooks for the recovered bools and constructors.
Keep FTableRowBase engine-owned, resolve enum/item/tag dependencies, and fail
closed on missing members or layout drift. Use normal promotion; never edit
`native/Source/FortniteGame/Generated/` by hand. Check emitted size/alignment and
reflected field offsets, not just successful type registration.

## Tables, item closure and count constraints

**data:** DefaultSolo's tables are `/Game/Athena/Playlists/AthenaCompositeLTD`
and `AthenaCompositeLP`. Parents are `/Game/Items/Datatables/AthenaLootTierData_Client`
and `AthenaLootPackages_Client`, plus `/Game/Athena/Playlists/OverrideLootTierData`
and `OverrideLootPackagesData`. Current inspector: composites have 175 / 1,361
rows; overrides have 0 / 2. All six extracted packages have uasset/uexp;
none of the six currently has either file at its corresponding staged path.
This updates the earlier observation in `generated/agents/loot/REPORT.md`.
Load the actual playlist's composites, preserving parent precedence; do not
append parents and overrides as independent weighted rows.

**data:** examples from the composite rows (not observed gameplay):

| Tier group / row suffix | Weight | NumLootPackageDrops | Package / constraints |
|---|---:|---:|---|
| Loot_AthenaFloorLoot / 13 | 1.3 | 1.6 | WorldPKG.AthenaLoot.Ammo; category-0 min 1 |
| Loot_AthenaFloorLoot_Warmup / 01 | 1 | 1 | shotgun package; category-0 min 1 |
| Loot_AthenaTreasure / 01 | .3 | 3 | HighShotgun; category mins [1,0,1,1,0,0,0,0,0,0,0] |
| Loot_AthenaAmmoLarge / 01 | 1 | 2 | WorldList.AthenaAmmoLarge; category-0 min 2 |
| Loot_AthenaAmmoSmall / 01,02 | 0,0 | 1,2 | neither row is a positive-weight candidate |

These samples have category weights [1,0,0,0,0,0,0,0,0,0,0], max entries -1,
and bonus enabled. Floor sample world bounds are -1/-1. AmmoLarge package
samples: shells Count=4 Weight=.5, light 18/.5, medium 10/1 (world level 1),
heavy 6/.75, rockets 2/.176. Do not silently cast 1.6 to 1 or replace empty
positive-weight selections with arbitrary ammo. The meaning of -1 sentinels,
fractional-drop rounding and category constraints needs an explicit contract.

**data:** conservative reference traversal starting at ALL rows of each tier
group (including weight zero), following LootPackageCall strings, found:
floor 36 package IDs / 216 distinct item soft paths; warmup 25 / 205;
treasure 28 / 166; AmmoLarge 1 / 5; AmmoSmall 1 / 5; no missing called IDs.
Counts overlap and include all world levels; they are not cooked dependency counts.
Use the union from the loaded, redirected groups to produce an exact staging
manifest, then follow each item's hard AND soft dependencies. Do not sum these
counts or stage one example weapon and call the closure complete.

Ammo roots include `/Game/Athena/Items/Ammo/AthenaAmmoDataShells`,
`AthenaAmmoDataBulletsLight`, `AthenaAmmoDataBulletsMedium`,
`AthenaAmmoDataBulletsHeavy`, and `AmmoDataRockets` in that same directory.
For every reachable WID, include AmmoData, WeaponStatHandle.DataTable and the
referenced weapon actor/ability dependencies. UFortWeaponItemDefinition exposes
WeaponStatHandle and soft AmmoData; FFortBaseWeaponStats contains ClipSize.
**inferred:** filling a newly rolled weapon to ClipSize is a proposed rule,
not recovered initial-loaded-ammo semantics. Dropped weapons must preserve their
actual loaded ammo; loose ammo Count and weapon LoadedAmmo are separate state.
Staging weapon abilities belongs to the shared combat owner; loot cannot prove
shoot/reload merely by placing a weapon in a quickbar.

## Real map actors and initialization

**data:** floor markers are Blueprints derived from native BuildingContainer:
`/Game/Athena/Environments/Blueprints/Tiered_Athena_FloorLoot_01` and
`Tiered_Athena_FloorLoot_Warmup` in the same directory.
Their groups are Loot_AthenaFloorLoot / Loot_AthenaFloorLoot_Warmup.
Both CDOs set bStartAlreadySearched_Athena=true, bDestroyContainerOnSearch=true,
SearchBounceRadiusOverride=200, Athena toss speed=350, cone half-angle=180,
and ATTACH_Floor. Do not turn these invisible markers into searchable chests.

**data:** `Apollo_Terrain_LS_Lobby` has no floor-marker exports;
`Apollo_POI_LobbyIsland_b` has 113 warmup-marker exports.
`Apollo_Terrain_Sub_E5` has four normal markers and four Tiered_Chest_Athena_C.
Example root-component relative positions: E5 FloorLoot_11
(-7917.219,8603.689,-768.556), FloorLoot_2 (11023.927,-3091.417,-754.670);
LobbyIsland Warmup100_155 (1511.953,-6439.891,-2688), yaw approximately -90.
These are sublevel-local transforms, not teleport coordinates. Resolve loaded
actor world transforms and level transforms before spawning or moving players.
Current stage has the standard marker and chest Blueprint packages, but lacks
the warmup marker and the sampled E5 / LobbyIsland_b maps; LS_Lobby is present.
Inventory actual loaded actors: a landscape-only stream cannot provide loot.

**binary:** container BeginPlay is `0x141bd5780` (primary vtable +0x328).
It calls base `0x141c30640`, checks authority and bSpawnedActor, uses mode gate
`0x141bd52c0`, then playlist/upgrade initialization `0x141bd5330`; it also calls
`+0xf10` (`0x141bf2a40`, client searched-mesh diagnostics). The upgrade helper
walks PotentialRandomUpgrades, evaluates scalable chance/enabled state and can
replace SearchLootTierGroup and ChosenRandomUpgrade. Preserve the gate and flags;
rd.py's offset comments select one member per byte and misname shared bit bytes.
Use bool_bits.json: +0x3e5 mask 1 is bWorldReadyCalled, not bUsesDayPhaseChange.
The +0xc4a bits 2/4 touched here are unreflected initialization state.

**binary:** `0x1419a8a20` compares the two floor group names and returns its
input bool only for those groups; it is a classification virtual, not a spawner.
No intact original server floor population/roll loop was established. Thus the
server-start enumeration below is explicitly reconstruction, not a recovered
AFortGameMode function. Trace WorldReady callers and Blueprint scripts before
claiming the original spawn trigger or spawn probabilities.

## Container interaction and pickup/drop boundary

**data:** chest `/Game/Building/ActorBlueprints/Containers/Tiered_Chest_Athena`
uses **Loot_Treasure**, not Loot_AthenaTreasure, so recover playlist redirects.
It has optional Loot_ApolloTreasure_Rare upgrade controlled by AthenaGameData
Default.ApolloChest.SpawnPercent / Enabled; Athena loot offset (0,50,30),
toss speed 600, bDestroyContainerOnSearch=true. Ammo box is
`/Game/Building/ActorBlueprints/Containers/Tiered_Ammo_Athena`.
Both chest and ammo-box uasset/uexp exist in stage; compatibility is unverified.
Native container defaults include Athena toss 550/cone 60 and loot tier -1;
apply native defaults before Blueprint overrides, not after them.

**binary/source:** interaction component wrapper `0x141d15810` calls real
`0x141d14c20`; see R4 section 3 for its validation/notification sequence.
Generated ServerAttemptInteract_Implementation is still empty. Bind its exec
`0x1428a5d20` or promote the body; preserve non-container interaction. Actor bounds
checks use 2*ValidationDistance plus bounds radius/height, then interface +0x68,
controller success notifications and container +0xaa8/+0xab0. Do not replace
this with an unconditional chest cast. Container ServerOnAttemptInteract chunk
`0x141be8654` is a further boundary/interface-target investigation lead, not a
validated function start. OnRep_bAlreadySearched exec is `0x14273c310`.
Its generated body is empty; bAlreadySearched is COND_Custom. Recover original
PreReplication activation and server search transition. Do not change the
replication condition globally or assert ForceNetUpdate bypasses it. The vanilla
client supplies its own OnRep visuals; the server must send the right state or
original destruction event, including late relevancy and repeated requests.

**binary:** pickup implementations are stripped. Execs: ordinary
`0x142a7ed80`, RequestedSwap `0x142a7ef10`, WithSwap `0x142a7f100`;
drop exec `0x142a4a130`. Use generated signatures for argument order:
Pickup, [Swap GUID], FlyTime, StartDirection, bPlayPickupSound;
drop takes ItemGuid, Count, bTrash. These are reconstruction boundaries.
Original supporting leads from D5: PickUpActor `0x14233f230`, AddItemToQuickBars
`0x142368650`, AddItem `0x142000d10`, RemoveItemByGuid `0x141efa680`,
RemoveFromQuickBars `0x1423a83a0`, TossPickup `0x14203cda0`.
PickUpActor's cached body delegates through an ability/interface path; it is not
proof of the missing inventory transaction. TossPickup has a real authority
wrapper, movement virtual +0x720 and update notification; recover callees too.
AFortPickup ctor recovery has DefaultFlyTime=1.5, trace enabled, NetDormancy=2,
bReplicates=true. Restore defaults/components before relying on flight visuals.

## Ordered work packages (one Native/generation/build owner)

All names below are **proposed new** implementation files/diagnostic log tags.
Existing FortAthenaLoot/InventoryFlow/Gameplay files already have live edits:
coordinate with their owner. Each package's runtime test uses the shared recipe
below at the next coherent, owner-approved integration checkpoint, not a build
per helper. Carry forward unaffected passing checks; research ran none.

### G8-L1 — emit compatible rows and native defaults

Own new athena_loot_runtime policy/contracts and narrow gen_native_classes hooks.
Implement the exact layouts/defaults above in order: package, tier, roots,
dependency closure. Original targets: setters, ops constructors, then registration
identities above. Assets: six tables. Validation: fail-closed promotion, all fields,
bool offsets, size/alignment and initialized default values; test recipe load log
`LootData: rows tier=175 package=1361` with both RowStruct paths correct.
Done: composites load without FallbackStruct/unknown row/member errors and
original omitted defaults survive. Declaration generation alone is insufficient.

### G8-L2 — playlist tables, redirects and complete content closure

Own new `Private/FortAthenaLootData.{h,cpp}` and generated agent staging manifest.
Read playlist LootTierData/LootPackages/LootLevel, redirects and game-feature
tables; retain loaded objects safely across GC/world teardown. Stage six tables,
floor/chest/box BPs, actual referenced map streams, recursive items and dependencies
using tools/stage_server_content.py --soft with an adequate package cap.
Original leads: game-mode properties SupportedAthenaLootTierGroups,
RedirectAthenaLootTierGroups, GameFeatureLootTierData/GameFeatureLootPackageData;
do not invent an ApplyPlaylistDataTables address. Inspect actual DefaultSolo
redirect entries before implementing Loot_Treasure resolution.
Test: `LootData: missing=0`, log redirect input/output and every failed soft load;
assert the union manifest resolves, including AmmoData and weapon stats.
Done: reproducible complete closure, no silent root limits or missing items;
offline extraction checks do not establish runtime compatibility.

### G8-L3 — explicit rolling contract and roll engine

Own new `Private/FortAthenaLootRoll.{h,cpp}`; route stripped Kismet entry points
and container/floor callers through one engine. Port no fictitious original body:
first document each unresolved semantic against independent 12.60 server evidence
if available, otherwise label the implementation stand-in and parity unresolved.
Resolve in order: redirects/world/tier/tag filters; positive effective weights;
named multipliers; tier choice; fractional total; category minima/weights/maxima;
recursive call Count; terminal item counts/levels; quota/streak and bonus rules.
Do not round 1.6, flatten package recursion, ignore fields, invent fallback loot,
or silently discard unsupported rows. Reject/report unsupported contracts.
Assets: L2 closure. Tests: deterministic injected RNG at selection boundaries,
fractional row, category minima/exhaustion, nested calls, zero candidates and
cycle/error handling; these test difficult contracts, not copied implementation.
Runtime: `LootRoll: group=... tierRow=... packageRow=... count=...` per decision,
plus explicit unsupported/missing failure. Done: implemented rules justified and
proven; unresolved original semantics remain a parity blocker, even if loot works.

### G8-L4 — floor population from actual loaded markers

Own new `Private/FortAthenaLootWorld.{h,cpp}` and a narrow Gameplay/MatchFlow hook.
Port initialization leads 0x141bd5780 -> 0x141bd52c0 -> 0x141bd5330 first;
trace original WorldReady/Blueprint paths and restore their called effects.
Reconstruction if the server trigger remains stripped: after playlist readiness,
enumerate loaded BuildingContainer marker subclasses; handle later level additions
exactly once per actor/world, choose warmup/main phase correctly, and roll at
actual world transforms using CDO toss data. Preserve original chance/upgrade
configuration; unknown probabilities must remain explicit blockers. Do not create
hardcoded item piles or rescan/spawn duplicates each tick/phase/stream event.
Assets: L2 plus both marker BPs and LobbyIsland_b/E5 streams or verified equivalent
loaded streams. Test: `LootWorld: markers normal=N warmup=M`, per-marker outcome
and `LootWorld: duplicate=0`; screenshots show naturally placed pickups. Done:
warmup and main island both populate, late streams work, no duplicate/hidden
container actors mistaken for loot; exact original startup trigger labeled honestly.

### G8-L5 — validated chest/box search and replication

Own new `Private/FortAthenaLootSearch.{h,cpp}` and generator-input bodies for
interaction/container behavior. Order: real 0x141d15810/0x141d14c20 validation,
resolve container interface handler and search setter/PreReplication, then replace
stripped 0x141bec430 through L3 and spawn through the common pickup initializer.
Preserve requests, range, callbacks, upgrades, destruction/mesh behavior and
exactly-once search. Do not mark searched merely because an unsupported roll failed.
Assets: chest/box plus selected redirected roll closure. Test recipe search twice
for each: one `LootSearch: committed`, one loot set, second request no new set;
opened/destroyed visual on vanilla client and correct late-relevancy state.
Done: both container types work, invalid interactions reject, other interactables
keep their existing behavior and COND_Custom has a justified activation path.

### G8-L6 — pickup lifecycle, inventory capacity and ammo

Own new `Private/FortAthenaLootTransfer.{h,cpp}`, coordinated narrow InventoryFlow
API changes and replace the limited FortAthenaLoot RPC binding. Implement shared
pickup initialization/toss and transfer transaction before binding all three
pickup RPCs. Port/review the real supporting helpers above in call order.
Plan stack capacity across allowed stacks/quickbars; return accepted and remainder
counts. Existing GiveItem returns a non-null existing stack after partial fit:
that return cannot authorize destroying the whole pickup. Handle multi entries,
partial pickup remainder, invalid targets and concurrent/double requests.
Replicate bPickedUp/PickupLocationData with target, direction/time/sound; defer
destruction until transfer/flight semantics permit it; rollback failed mutations.
Preserve full entry state, sync equipped weapon ammo, distinguish loose ammo from
LoadedAmmo. Assets: roll closure/pickup movement dependencies. Test ordinary weapon,
auto ammo, nearly full stack, full bar, duplicate request and failed transfer;
`LootTransfer: accepted=A remainder=R` must conserve counts and HUD matches.
Done: no loss/duplication, correct visible flight/quickbar and loaded/reserve ammo;
unknown original validation/timing rules are explicitly tracked, not called ported.

### G8-L7 — requested swap and inventory drop

Own transfer module extensions and scoped InventoryFlow quickbar/removal integration.
Recreate stripped swap/drop RPCs from their exact signatures, using shared L6
transaction. Validate swap ownership/current slot and item droppability/count;
respect bTrash separately. Allocate outgoing pickup before committing removal,
preserve outgoing ammo/level/state, update replicated entries, GUID index, quickbar
and equipped weapon together; rollback allocation/transfer failure. Reuse real
TossPickup after its callees are ported. Do not give the outgoing pickup a stale
inventory identity accidentally; define identity changes against original evidence.
Assets: outgoing item closure. Test full-bar requested swap, partial ammo drop,
full weapon drop, re-pickup, invalid GUID/count and failed spawn. Log incoming /
outgoing GUIDs, slot, counts and LoadedAmmo. Done: correct same-slot replacement,
HUD removal, visible outgoing pickup, conserved total ammo and no stale equipped
weapon/duplicate RPC transfer. These are recreated server rules until proven.

## Shared vanilla-client acceptance recipe

Implement opt-in `-FortniteLootTest=chest|ammo|pickup|swap|drop` in a new
`Private/FortAthenaLootTest.cpp` as part of the integration batch. This is a
**proposed fixture**, not an existing flag. After all prerequisites load, select
an actual map actor/natural pickup, position and face the possessed pawn at it,
and emit `LootTest: ready`. No hardcoded replacement loot may earn floor coverage.
Use long warmup for interaction tests; separately run ordinary bus/landing with
the fixture disabled to prove main-island population. Fixture must fail loudly
when no real actor loads. Log its selected actor, world transform and tier group.

At the next build/runtime checkpoint, the single Native owner builds/stages once,
then runs (change label/scenario per case; commands below are prescribed, not run):

```sh
FN_RUN_LABEL=g8-loot-chest FN_RENDERER=dxvk FN_LP_NUM_THREADS=4 \
FN_SERVER_MAP=/Game/Athena/Apollo/Maps/Apollo_Terrain \
FN_SERVER_URL_OPTS='?RequiredPlayers=1' \
FN_SERVER_ARGS='-FortniteSkipUnconvertedLegacyShaderMaps -FortniteTolerateNeedLoadDependencies -FortniteSkipEngineClassNetChecksum -FortniteWarmupSeconds=900 -FortniteLootTest=chest' \
FN_ACTIONS='0:wait,UI.State.Startup.SubgameSelect,900 20:click,640,592 0:wait,ShouldShowBeginningOfSeasonDialog,600 25:click,1144,473 0:wait,LootTest:.ready,900 0:shot 1:hold,e,3 5:shot 7:hold,e,3 11:shot' \
porting/tools/vanilla_play_run.sh 1800
```

Ammo-box search uses the same actions and `=ammo`. Pickup uses `=pickup` and
the suffix after readiness `0:shot 1:hold,w,1 3:hold,e,2 6:shot` (fixture must
put natural ammo in the walking path and face the natural weapon). Swap uses
`=swap` and `0:shot 1:hold,e,3 6:shot`, with fixture preparing a full bar and
recording the intended swap slot. Drop uses `=drop` and candidate default-bind
suffix `0:key,Tab 2:shot 3:key,x 5:shot 7:key,Tab 9:shot`; first verify the
client's selected slot/drop binding and adjust from the recorded inventory screen.
The X sequence is not evidence until ServerAttemptInventoryDrop is observed.
If the UI needs a count dialog, record and supply the exact click/key sequence
from that screen, then repeat partial/full-drop cases. Do not substitute a
server-injected drop RPC for a client-UI acceptance claim.

All LootData/LootRoll/LootWorld/LootSearch/LootTransfer/LootTest tags above are
required NEW instrumentation. A wait timeout continues the action script, so
acceptance must explicitly assert readiness and the requested RPC, not screenshots
alone. Save source/build/run identifiers, server/client logs and screenshots in
the standard generated/builds and generated/runs paths. Require no new missing
package, NetChecksum, NetGUID or actor-spawn failures. L1–L7 source is build
pending until this combined checkpoint. Tests prove only the paths exercised;
loot completion alone does not pass the entire G8 combat/build/elimination gate.
