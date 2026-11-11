// CONFIDENCE: MEDIUM Original addresses: thunk 0x142b47010; see evidence.json for callees.
// Signature: TSoftObjectPtr<UTexture2D> GetWeaponDataSmallPreviewImage()
if (WeaponData)
{
    // Helper proposed: NativeGetSmallPreviewImage (vtable slot 0x318 on WeaponData).
    return WeaponData->NativeGetSmallPreviewImage();
}
UE_LOG(LogFort, Warning, TEXT("Weapon [%s] - Invalid WeaponData in GetWeaponDataSmallPreviewImage"), *GetName());
return TSoftObjectPtr<UTexture2D>();
