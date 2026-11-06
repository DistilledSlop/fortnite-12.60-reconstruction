// CONFIDENCE: HIGH
// Primary AFortWeapon virtual slot +0x6a8, 0x140696570.
const UFortWeaponItemDefinition* Definition = WeaponData;
return Definition ? Definition->WeaponStatHandle.GetRow<FFortBaseWeaponStats>(
    TEXT("UFortWeaponItemDefinition::GetWeaponStats")) : nullptr;
