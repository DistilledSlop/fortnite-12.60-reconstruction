// CONFIDENCE: HIGH - null-data warning, empty-input early out and tag overlap test are exact (0x1426bb650); the tag source matches the ported GetWeaponDataGameplayTags path.
// Original addresses: exec 0x142b48ec0 -> native 0x1426bb650.
if (!WeaponData)
{
    UE_LOG(LogFort, Warning, TEXT("Weapon [%s] - Invalid WeaponData in WeaponDataHasAnyMatchingGameplayTags"), *GetName());
    return false;
}
if (TagContainer.Num() < 1)
{
    return false;
}
FGameplayTagContainer WeaponTags;
GetWeaponDataGameplayTags(WeaponTags);
return WeaponTags.HasAny(TagContainer);
