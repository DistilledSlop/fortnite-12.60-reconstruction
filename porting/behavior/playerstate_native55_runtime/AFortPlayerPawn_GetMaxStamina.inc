// CONFIDENCE: HIGH
// Reflected exec 0x142a7c490; native 0x142321e10.
const AFortPlayerState* FortState = Cast<AFortPlayerState>(GetPlayerState());
if (FortState && FortState->AttributeSets.PlayerAttrSet)
{
    const UFortPlayerAttrSet* PlayerAttrSet = FortState->AttributeSets.PlayerAttrSet;
    return PlayerAttrSet->MaxStamina.GetCurrentValue();
}
return 0.0f;
