# Creator component flags and attribute registration

This is a narrow source mapping, not a complete creator implementation. The remaining lifecycle and setting blockers in building_asc54_runtime/HANDOFF.md still apply.

Native creator `porting/generated/callee_probe/building_vitals_dependencies_3/0x141b94070.asm` lines at 0x141b94157–0x141b9417b clears masks 04, 02 at component+0x3a and 80 at +0x85, then calls 0x1440c2bb0. Source mapping, in order:

```
ASC->PrimaryComponentTick.bStartWithTickEnabled = false;
ASC->PrimaryComponentTick.bCanEverTick = false;
ASC->bAutoActivate = false;
ASC->RegisterComponent();
```

Evidence: target `porting/generated/reflection/fortnite_game_reflection.json` records UActorComponent PrimaryComponentTick at 48 (0x30). `struct_blocks.json` records native FTickFunction TickGroup at +8, EndTickGroup at +9 and the reflected tick flags in the same order as Engine 4.26 `Engine/Source/Runtime/Engine/Classes/Engine/EngineBaseTypes.h:196` (bTickEvenWhenPaused, bCanEverTick, bStartWithTickEnabled); the target byte at +0x3a is therefore the tick flags byte. The mapping of the two masks follows the 4.26 bitfield declaration/packing and target struct layout; the extracted struct bit properties do not themselves record masks, so this is source-layout inference rather than independent native named-bit proof. For bAutoActivate, target `porting/generated/reflection/bool_bits.json` independently records UActorComponent offset 133 (0x85), mask 128. These members are public in 4.26. Direct bAutoActivate assignment preserves the unconditional native store: SetAutoActivate is virtual and includes registration/construction checks (`ActorComponent.cpp:1814`), so it is not an unconditional equivalent.

`0x1440c2bb0` (`building_vitals_dependencies_4/0x1440c2bb0.c`) obtains owner at +0xa0, invokes owner GetWorld vslot +0x150, and calls 0x1440c2ce0 with component and world if nonnull. This matches 4.26 `UActorComponent::RegisterComponent` (`ActorComponent.cpp:1270`) in its successful path; source also calls ensure on null world. Use the public `RegisterComponent()` API; null-world diagnostic differences and any underlying engine version differences should not be silently equated.

Creator 0x141b94356–0x141b943cf obtains ASC +0x140 through 0x1406ccde0 (`_4/0x1406ccde0.c`), searches for the attribute-set pointer, and appends only if absent. Target reflection `fortnite_game_reflection.json` identifies replicated UAbilitySystemComponent SpawnedAttributes at offset 320 (0x140), array of UAttributeSet. The source 4.26 member is public (though deprecated); its direct `ASC->SpawnedAttributes.AddUnique(BuildingAttributeSet)` reproduces the observed native accessor + pointer AddUnique without extra observed operations. Public `GetSpawnedAttributes_Mutable().AddUnique(...)` and `AddAttributeSetSubobject(...)` add a property-dirty mark in 4.26 (`AbilitySystemComponent.cpp:2669`, `AbilitySystemComponent.h:137`); the native accessor only returns this+0x140, even when an existing pointer is found. Thus those APIs change behavior and are not proven exact. Direct member access may produce a deprecation warning.

On the other policy branch the helper bodies `_4/0x1440cb120.c` and `_4/0x14072fb70.c` set ASC+0x84 mask 0x10 and AttributeSet+0x28 mask 1 respectively. Target bool_bits identifies component bNetAddressable as +0x84 mask 0x10. Engine 4.26 `ActorComponent.cpp:1881` and `AttributeSet.cpp:368` define the public `SetNetAddressable()` methods as setting their respective bNetAddressable flags. The AttributeSet identification follows source semantics and helper body; it is not independently named by the available target bool metadata. Both objects become stable by name for networking; this does not itself enable replication.

No canonical files, tests, builds or commits changed. No Ghidra used.
