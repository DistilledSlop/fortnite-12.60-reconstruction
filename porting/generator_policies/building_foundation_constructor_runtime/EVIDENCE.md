# Foundation constructor: selected fields

GPSC -> InternalConstructor -> `ABuildingFoundation` constructor `0x141bce060` (using `porting/tools/ctor_defaults.py`). The base constructor call is at `0x141bce077`; these assignments are appended to the existing generated constructor, preserving its base initialization and any default-subobject creation. The projection is limited to named fields below; it is not a foundation state or streaming implementation, and does not claim constructor equivalence.

Original reflection offsets and bool masks come from `fortnite_game_reflection.json`, `class_parent_map.json` and `bool_bits.json`. Declarations and enum values come from generated class/enum headers, and Actor fields from UE 4.25's `Actor.h`. The referenced inherited fields are public; bitfields are updated individually, preserving unrelated bits.

| Original store | Reflected field(s) and exact operation |
| --- | --- |
| 0x141bce3e1 | inherited bRotateInPlaceEditor = false (AND 0xf7 at 0x3e5) |
| 0x141bce3e8 | inherited bAlwaysRelevant = true (OR 0x10 at 0x58) |
| 0x141bce427, 0x141bce42e | first two 32-bit components of inherited BaseLocToPivotOffset and SnapOffset = zero; third components untouched |
| 0x141bce435 | FoundationType = 3, BFT_None (byte at 0xb6c) |
| 0x141bce43c -> 0x141c1c320 | inherited bNoCollision = true (helper clears mask 8 then ORs 1 << 3 at 0x6a8) |
| 0x141bce441..0x141bce45d | byte at 0xb68 RMW AND 0xd6 OR 0x14: bConditionalFoundation=false, bOverrideNavigationGraphCells=true, bHasExcludedZone=false, bForceDitheringTransition=true, bStreamingDataBasedBounds=false; bit 0x02 is preserved |
| 0x141bce453 | inherited NetPriority = float 10 (0x41200000 at 0x110) |
| 0x141bce46c | inherited DynamicBuildingPlacementType = 2, DestroyAnythingThatCollides (byte at 0x396) |
| 0x141bce475 | inherited AbilitySystemComponentCreationPolicy = 1, Lazy (byte at 0x34d) |
| 0x141bce463..0x141bce47c | byte at 0xd84 RMW AND 0xf9 OR 1: bMergeHLODs=true, bExcludeFromHLODMerge=false, bHLODMergeUseCustomTextureSize=false |
| 0x141bce489 | ProxyMeshMaxDrawDistanceMultiplier = float 1 (0x3f800000 at 0xd80) |
| 0x141bce493 | HLODMergeCustomTextureSize = (0,0) (8-byte zero at 0xd88; reflected FIntPoint) |
| 0x141bce4c6 | inherited bReplayRewindable = true (OR 0x10 at 0x59) |
| 0x141bce570 | inherited bCanBeSavedInCreativeVolume = false (AND 0xfb at 0x3e4) |

Additional original constructor operations are deliberately unresolved or outside this projection. They include initialization of reflected containers, names, text, `LandIcon` (including nonzero values), `StreamingData`, `DynamicFoundationRepData` and `DynamicFoundationTransform`; a series of non-reflected fields and bit writes at 0xb69/0xd30/0xed8; a value loaded from a global into 0xb80; a function call on the existing StaticMeshComponent followed by a component bit change; a global/material lookup assigned to HLODMergeBaseFlattenMaterial; another helper and global counter increment. These operations and any corresponding side effects are not reconstructed here. In particular, this policy does not reset `RootComponent`, attach components, or replace the existing default-subobject constructor. Zero bytes in unrelated, non-reflected storage are not mapped to guessed field names.

AFortWorldSettings constructor `0x141fa4800` also explicitly initializes reflected
`SplineHLODIndex` at +0x3cc to -1 (`0x141fa4edd`). This policy projects that single
field as an in-class CDO initializer; serialized package values override it.
It does not claim the rest of that constructor. The default is required by the
foundation proxy visibility path `0x141bea280`, which compares streamed HLOD actor
levels to `SplineHLODIndex + 1`. Original code bytes are pinned by the world-manager
lifecycle evidence windows.
