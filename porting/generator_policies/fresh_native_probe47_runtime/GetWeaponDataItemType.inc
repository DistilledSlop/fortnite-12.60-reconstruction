// Native callee 0x1426a0b10; CONFIDENCE: HIGH.
if (WeaponData)
{
    return WeaponData->NativeGetItemType();
}
UE_LOG(LogFort, Warning, TEXT("Weapon [%s] - Invalid WeaponData in GetWeaponDataItemType"), *GetName());
return EFortItemType::Max_None;
