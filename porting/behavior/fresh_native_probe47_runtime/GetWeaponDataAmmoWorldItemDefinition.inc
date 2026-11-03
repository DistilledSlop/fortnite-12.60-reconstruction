// Native callee 0x1426a09e0; CONFIDENCE: HIGH.
if (WeaponData)
{
    return WeaponData->GetAmmoWorldItemDefinitionNative();
}
UE_LOG(LogFort, Warning, TEXT("Weapon [%s] - Invalid WeaponData in GetWeaponDataAmmoWorldItemDefinition"), *GetName());
return nullptr;
