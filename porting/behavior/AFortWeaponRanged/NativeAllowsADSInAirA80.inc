// CONFIDENCE: HIGH. Complete original body; class lifecycle acceptance remains separate.
// Native AFortWeaponRanged virtual +0xa80, original 0x1426c07c0.
const UFortWeaponRangedItemDefinition* RangedData = Cast<UFortWeaponRangedItemDefinition>(WeaponData);
return RangedData != nullptr && RangedData->bAllowADSInAir;
