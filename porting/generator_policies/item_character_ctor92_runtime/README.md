# Narrow early constructor projections

`finalize` restores three explicit UFortAccountItem constructor writes: Quantity=1 at +0xb0, level=1 at +0xb8, and bIsLocalOnlyItem=true at +0xb4. Constructor 0x141ffb050 calls its parent and repeats these writes. Its raw decompile and the original property's set-bit function 0x1429e7be0 are in `porting/generated/agents/item-character-ctor92/`. The latter verifies the byte offset that reflection does not expose. `porting/generated/agents/k1-defaults/ctor_defaults.json` independently records the fields and instruction addresses.

UCustomCharacterPart 0x141d6b93c stores the packed word 0x0701 at +0x30: GenderPermitted=1 and BodyTypesPermitted=7. See `porting/generated/native129-constructors/0x141d6b6c0.asm`. Casts preserve the native byte values. The policy validates the field declarations, reflected offsets and parents. Call `finalize` for all classes, including UFortPersistableItem, to validate inherited member declarations. It projects only these selected writes and does not claim to port the complete constructors or their other defaults.

Run `python3 porting/generator_policies/item_character_ctor92_runtime/test_policy.py` to validate against actual reflection and generated fixtures, including conflict and field-offset rejection.
