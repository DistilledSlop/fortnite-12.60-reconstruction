// Native callee 0x1426a0ec0; CONFIDENCE: HIGH.
if (WeaponData)
{
    return WeaponData->NativeGetTriggerType();
}
UE_LOG(LogFort, Warning, TEXT("Weapon [%s] - Invalid WeaponData in GetWeaponDataTriggerType"), *GetName());
return EFortWeaponTriggerType::OnPress;
