// Original address: 0x1426a0f60 (thunk 0x142b47150). Offset evidence: WeaponData is UFortWeaponItemDefinition* at +0x350 (reflected AFortWeapon.WeaponData); the non-null path calls 0x142672cc0 = UFortWeaponItemDefinition::GetWeaponStatHandle() const, which fills the returned struct; the null path zeroes both qwords of the return buffer.
// Verbosity-gated diagnostic log at 0x142c54b70 elided (no state change)
// CONFIDENCE: MEDIUM
if (WeaponData != nullptr)
{
    return WeaponData->GetWeaponStatHandle();
}
return FDataTableRowHandle();
