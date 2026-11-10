// CONFIDENCE: MEDIUM
// Original address: 0x142a1b6d0 (exec thunk), native helper 0x1423150b0 (dep AFortPawn__EquipWeaponDefinitionImpl, PARTIAL)
// Script read order: WeaponData (object), then ItemEntryGuid (FGuid, copied by value). Return value is the equipped weapon object.
ReturnValue = EquipWeaponDefinitionImpl(WeaponData, ItemEntryGuid);
