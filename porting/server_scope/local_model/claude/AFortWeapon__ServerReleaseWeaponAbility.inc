// CONFIDENCE: HIGH - RPC validate-then-dispatch shape is exact; the implementation body is MEDIUM (dep AFortWeapon_ServerReleaseWeaponAbility_Implementation).
// Original addresses: exec 0x142b48150; validate vtable +0x628 -> 0x1406801d0; implementation vtable +0x630 -> 0x1426b0cb0 -> 0x1426aec00.
if (!ServerReleaseWeaponAbility_Validate(SpecHandle))
{
    RPC_ValidateFailed(TEXT("ServerReleaseWeaponAbility_Validate"));
    return;
}
ServerReleaseWeaponAbility_Implementation(SpecHandle);
