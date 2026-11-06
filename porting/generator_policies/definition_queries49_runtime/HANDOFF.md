# Definition and weapon queries

Four proposed bodies close two connected queries. The source names `NativeGetRarity` and `NativeGetTriggerType` are assigned, with proposed const qualification. The rarity operation is virtual so that weapon calls preserve subclass overrides. This covers the base target, not the implementations of every derived override.

* ItemDefinition and WorldItemDefinition constructor-installed tables at 0x14606bf00 and 0x14606f8f8 both select 0x1409dd610 at slot +0x2a8. That target returns the byte at +0x50, reflected `Rarity` (`EFortRarity`).
* Weapon native 0x1426a0c40 tests its reflected WeaponData at +0x350 and tail-dispatches that definition slot. On null it logs the exact warning and returns byte 7, `Unattainable`. Reflected wrapper is 0x142b46fe0 (not 0x142b46f20), flags 0x54020401.
* Definition helper 0x142671d20 copies the byte at +0x9a4 into return storage. The reflected field at that offset is `UFortWeaponItemDefinition::TriggerType`, type `TEnumAsByte<EFortWeaponTriggerType::Type>`.
* Weapon native 0x1426a0ec0 calls that nonvirtual helper on WeaponData; on null it logs the exact warning and writes zero (`OnPress`) to return storage. Reflected wrapper is 0x142b47110, flags 0x54020401. The local source uses a named helper to retain the call's ownership and semantics.

Native evidence pins target slots, bodies and wrappers. Constructor-installed table identifications are inherited from `item_definition_key_runtime` and `worlditem_dispatch_contracts_runtime`; reflection bindings are in `porting/generated/reflection/fortnite_game_reflection.json`, and wrapper disassembly is recorded in `fresh_native_probe47_runtime/wrapper-evidence.json`. No build, tests, runtime execution, canonical changes or commit were made.
