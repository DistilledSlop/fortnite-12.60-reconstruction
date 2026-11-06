// CONFIDENCE: HIGH
// Reflected exec 0x142a7ca10; native 0x142324c10.
const AFortPlayerState* FortState = Cast<AFortPlayerState>(GetPlayerState());
if (FortState && FortState->AttributeSets.PlayerAttrSet)
{
    const UFortPlayerAttrSet* PlayerAttrSet = FortState->AttributeSets.PlayerAttrSet;
    return PlayerAttrSet->Stamina.GetCurrentValue();
}
return 0.0f;
