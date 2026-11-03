// Native callee 0x1426a0960; CONFIDENCE: HIGH.
if (WeaponData)
{
    return WeaponData;
}
UE_LOG(LogFort, Warning, TEXT("Weapon [%s] - Invalid WeaponData"), *GetName());
return nullptr;
