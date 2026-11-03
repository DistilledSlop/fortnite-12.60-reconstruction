// CONFIDENCE: HIGH
// Reflected exec 0x142a7c0e0; native 0x14231b600.
const AFortPlayerState* FortState = Cast<AFortPlayerState>(PlayerState);
if (FortState && FortState->AttributeSets.PlayerAttrSet)
{
    const UFortPlayerAttrSet* PlayerAttrSet = FortState->AttributeSets.PlayerAttrSet;
    const float Value = PlayerAttrSet->ExtraLives.GetCurrentValue();
    // Match CVTTSS2SI for NaN and values outside the signed 32-bit range.
    if (!(Value >= -2147483648.0f && Value < 2147483648.0f))
    {
        return MIN_int32;
    }
    return static_cast<int32>(Value);
}
return 0;
