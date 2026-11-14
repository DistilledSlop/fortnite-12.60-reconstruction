// CONFIDENCE: HIGH
// Original 0x1426930a0; +0x828 virtual.
AFortPawn* Pawn = Cast<AFortPawn>(GetInstigator());
const int32 Capacity = NativeGetBulletsPerClip14269b000();
if (!WeaponData || !WeaponData->GetAmmoWorldItemDefinitionNative() || Capacity < 1)
{
    return false;
}
const auto Fail = [FailureTags](uint32 Offset) -> bool
{
    if (FailureTags)
    {
        FailureTags->AddTag(FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(Offset));
    }
    return false;
};
if (NativeIsReloadAbilityActive830())
{
    return Fail(0x9b8);
}
if (!Pawn)
{
    return false;
}
if (!Pawn->NativeCanPerformAction14230c260(
        FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x6d0)))
{
    return Fail(0x9c0);
}
if (NativeGetReserveAmmo7D8() < 1)
{
    return Fail(0x9c8);
}
if (GetMagazineAmmoCount() >= Capacity && GetLocalMagazineAmmoCount() >= Capacity)
{
    return Fail(0x9d0);
}
const float PreviousReloadTime = LastSuccessfulReloadTime;
if (!((GetWorld()->TimeSeconds - PreviousReloadTime) * CustomTimeDilation > 0.4f))
{
    return false;
}
if (bCheckExtra && NativeIsFiring708())
{
    return Fail(0x9d8);
}
return true;
