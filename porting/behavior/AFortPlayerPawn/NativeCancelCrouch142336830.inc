// CONFIDENCE: HIGH
// Original 0x142336830.
if (!GetCharacterMovement() || IsMoveInputIgnored())
{
    return;
}
if (GetCharacterMovement()->bWantsToCrouch)
{
    UnCrouch(false);
}
else if (CanCrouch())
{
    Crouch(false);
}
AFortWeaponRanged* RangedWeapon = Cast<AFortWeaponRanged>(CurrentWeapon);
if (RangedWeapon)
{
    const bool bUsesFirstShotAccuracy = RangedWeapon->NativeUsesFirstShotAccuracy1426cfa80();
    const bool bHasFirstShotAccuracy = RangedWeapon->NativeHasFirstShotAccuracy1426d2660();
    if (bUsesFirstShotAccuracy && bHasFirstShotAccuracy)
    {
        RangedWeapon->NativeClearFirstShotAccuracy1426ddc90();
    }
}
