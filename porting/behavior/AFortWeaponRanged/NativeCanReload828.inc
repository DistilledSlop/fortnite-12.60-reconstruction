// CONFIDENCE: HIGH
// Original 0x1426c49c0; +0x828 virtual override.
const UFortWeaponRangedItemDefinition* RangedData = Cast<UFortWeaponRangedItemDefinition>(WeaponData);
if (RangedData && RangedData->DoNotAllowDoublePump() && NativeNeedsToBeCocked)
{
    return false;
}
return AFortWeapon::NativeCanReload828(bCheckExtra, FailureTags);
