# building_vitals_runtime (ABuildingActor vitals; native-only source batch)

Status: READY_MANIFEST.json = provenance of the 8 bodies the root integrated (5 plain + 3 native events; the earlier "14" was superseded); DRAFT_MANIFEST.json = GetCentroid (MEDIUM, drafts/) and GetMaxHealth (blocked). None canonical until primary integration.
Not built, not tested, no Ghidra opened (evidence: callee_probe/building_vitals_1 wrappers + read-only image disassembly).

Vtable chain: gpsc 0x142732ac0 -> thunk 0x1427339e0 -> ctor 0x141b86f60 -> single vptr store 0x145efb310
(529 code slots; build_vtable_slots.ctor_of/primary_vptr; the shared candidate 0x1459c1798 was not used).
Slots read from that table: 0x648 ShouldDie, 0x658 ShouldDisplayDamageNumbers, 0x660 UseDefaultHealthBar (all 0x1406801d0 = return true),
0x668 CurrentBuildingLevel 0x141b9d290, 0x7a8 Health 0x141b9edb0, 0x7b0 MaxHealth 0x141ba73d0, 0x7b8 Shield 0x141ba9860,
0x7c0 MaxShield 0x141ba75a0, 0x7c8 ShieldPercent 0x141ba9a70, 0x7d0 HealthPercent 0x141b9ef10, 0x8b8 Centroid 0x141b9c710.

Fields: ABuildingActor +0x318 BuildingAttributeSet (UFortBuildingActorSet : UFortHealthSet), +0x308 CurrentBuildingLevel,
+0x350 OwnerPersistentID (int16), bPlayerPlaced = byte 0x3e5 mask 0x20, +0x500 CentroidOffset.
UFortHealthSet attribute data: Health +0x30, MaxHealth +0x58, CurrentShield +0x80, Shield +0xa8 (max shield); read through the
tail-jumped 0x14070cc00 = `movss 0xc(%rcx)` = FGameplayAttributeData::GetCurrentValue.

Integration dependencies:
* The originals are virtual (slots above). The generator emits no virtual qualifier, so internal calls are plain member calls;
  derived-class overrides will not dispatch until the primary declares them virtual.
* ABuildingActor.cpp needs `#include "UFortBuildingActorSet.h"` (header only forward-declares it) for GetHealth/GetShield/GetMaxShield.
* GetCentroid is MEDIUM: arithmetic matches FQuat::RotateVector, not byte-compared.

## Policy requirements
* All 14 ready methods are virtual in the original (slots listed above); root virtual support must declare them virtual in ABuildingActor so GetHealth/IsDamaged/percent bodies dispatch dynamically (GetMaxHealth, GetHealth, GetShield, GetMaxShield, GetShieldPercent, GetHealthPercent at minimum).
* ABuildingActor.cpp must include "UFortBuildingActorSet.h" (GetHealth/GetShield/GetMaxShield). GetHealth falls back to virtual GetMaxHealth, which is not yet recovered.
* ShouldDie params resolved by class_ref_map: EventInstigator AController*, DamageCauser AActor*.

## Blockers
| Method | Status |
|---|---|
| GetMaxHealth 0x141ba73d0 | drafts/*.inc.draft has the full flow. Resolved: 0x141b9b2a0 = GetAttributeInitializationKey(byte), 0x1406c6ef0 = IGameplayAbilitiesModule::Get()->GetAbilitySystemGlobals (slot 8), 0x14070c720 = globals+0x220 (initter), 0x141ba74f0 = lazy static MaxHealth attribute. Open: 0x141ed2350, 0x141ed4e30 and the initter's custom vtable slot 0x30 (not Engine ApplyAttributeDefault), plus reflected names of the key (+0x32c) and slot 0x778. |
| SetHealth 0x141bc4a90 | Shape: attr = 0x141916570 (Health attribute); asc = 0x14070f1a0(0x141b9b750(this)); if asc: 0x1406da850(asc, attr, value). Likely UAbilitySystemComponent::SetNumericAttributeBase, unconfirmed. |
| ForceBuildingHealth 0x141b990d0 | if Role(+0xf0)==3: lazily create the set (0x142ecc600, 0x141b94070 when 0x141ed2350()+199 and +0x34d==1), then set MaxHealth attr (0x141ba74f0) and Health attr (0x141916570) to value, in that order. Same unresolved helpers as SetHealth. |
| GetOwningController 0x141ba8530 / GetOwningPlayerState 0x141ba8b20 | typed C present; unresolved helpers 0x14468fb70, 0x142ee5dd0, 0x1423881c0, 0x142a85850, 0x1428ea8f0; not attempted. |
| GetCentroid | MEDIUM draft in drafts/; arithmetic not byte-compared. |
Next request (10 addresses): /tmp/claude-native-building-vitals/probe-request-2.json (also OPTIONAL_PROBE_REQUEST.json).

## Pass 3 (dependencies_2)
Identities: 0x1406da850 = UAbilitySystemComponent::SetNumericAttributeBase (ActiveGameplayEffects at +0x838 SetAttributeBaseValue, bIsNetDirty bit 2 at +0xbc; matches Engine source). 0x14070f1a0 = UAttributeSet::GetOwningAbilitySystemComponent (MEDIUM: Outer +0x20 interface lookup, vcall +0x10). 0x141916570 / 0x141ba74f0 = lazy static Health / MaxHealth FGameplayAttribute accessors (FindField on the health-set class, 0x1406a8e40 + 0x1406ebb90 ctor). 0x142ecc600 = UObject::IsTemplate (outer-chain flag 0x30). 0x141ed2350 = checked cast of GEngine (+0x288) to UFortEngine. 0x141b9b750 = get-or-create BuildingAttributeSet (+0x318); creation goes through 0x141b94070 (NewObject ASC "AbilitySystemComponent" at +0x358 and "BuildingAttributeSet"), not recovered. 0x141ed4e30 (GetMaxHealth) = world/game-state lookup returning a byte or 2.
Manifests: READY_MANIFEST.json (5 plain + 3 native events needing _Implementation support); DEPENDENCY_BLOCKED_MANIFEST.json (6 health queries blocked on GetMaxHealth; SetHealth, ForceBuildingHealth blocked on 0x141b94070); DRAFT_MANIFEST.json (GetCentroid MEDIUM, GetMaxHealth). Virtual flag only on proven vtable slots (9); GetMissingHealth, GetShieldOrHealthPercent, HasHealthLeft, IsDamaged, IsPlayerBuilt are direct callees and carry virtual=false.

## Pass 4 (dependencies_3), source-only, nothing built or tested
NEW_READY_MANIFEST.json is empty: no new body was fully closed.
* 0x141ed4e30 (GetMaxHealth byte) is closed as flow: world = GEngine->GetWorldFromContextObject(actor, error mode 0) (0x144640410, UEngine); if world->GameState(+0x188) casts to the game-state class (0x1428ea7f0) return byte at GameState+0x828 (0x141e5f160); else if GEngine+0x2a8 (AssetManager) is null return 2, else checked cast to UFortAssetManager (0x141df62a0, log "Invalid AssetManager class used, must be UFortAssetManager") and return byte at +0x6cc. Blocked as source: the three bytes (GameState+0x828, UFortAssetManager+0x6cc, UFortEngine+199) have no reflected names, and the game-state class identity is unproven.
* 0x141b9b2a0 GetAttributeInitializationKey: complete flow in drafts, but the mismatch log needs key-to-string 0x141bc8220 and the 16-byte key field (+0x32c) is unnamed.
* 0x141b94070 creator: still blocked. It creates the ASC (NewObject, class getter 0x14277e480, flags/clears on the new object, 0x1440c2bb0) and BuildingAttributeSet (class from 0x142851150 via param_1[0x62]), registers the ASC in a global array (0x1406ccde0) when policy==1, else calls 0x1440cb120/0x14072fb70, then virtual slots 0x790 (base: no-op 0x1406801c0) and 0x798 (0x141b8f710). Class identities and 0x798 body are unknown. SetHealth/ForceBuildingHealth and GetMaxHealth stay blocked on it.
* Engine identity added: 0x142ecc600 == UObject::IsTemplate(RF_ClassDefaultObject|RF_ArchetypeObject) (outer-chain flag walk).
* GetMaxHealth also still needs the initter virtual slot 0x30 target (custom, not Engine).
Slot targets read from vtable 0x145efb310: 0x778 -> 0x141b9b480, 0x790 -> 0x1406801c0, 0x798 -> 0x141b8f710, 0x7a0 -> 0x1406801c0.
Next request: probe-request-4.json (10 addresses, also OPTIONAL_PROBE_REQUEST.json).
