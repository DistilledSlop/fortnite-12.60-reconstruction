// Native callee 0x1426a0c40; CONFIDENCE: HIGH.
if (WeaponData)
{
    return WeaponData->NativeGetRarity();
}
UE_LOG(LogFort, Warning, TEXT("Weapon [%s] - Invalid WeaponData in GetWeaponDataRarity"), *GetName());
return EFortRarity::Unattainable;
