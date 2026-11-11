// CONFIDENCE: MEDIUM Original addresses: thunk 0x142b46e30; see evidence.json for callees.
// Signature: void GetWeaponDataGameplayTags(FGameplayTagContainer& GameplayTags)
if (WeaponData)
{
    // The native calls the interface at vtable slot 2 (IGameplayTagAssetInterface::GetOwnedGameplayTags).
    if (IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(WeaponData))
    {
        TagInterface->GetOwnedGameplayTags(GameplayTags);
    }
    return;
}
UE_LOG(LogFort, Warning, TEXT("Weapon [%s] - Invalid WeaponData in GetWeaponDataGameplayTags"), *GetName());
