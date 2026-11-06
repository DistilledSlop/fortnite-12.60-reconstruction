// CONFIDENCE: HIGH
// Native body 0x1423532a0; reflected exec 0x142a20d90.
if (GetLocalRole() == ROLE_Authority)
{
    if (bAdjustForInputYawScale)
    {
        const AFortPlayerController* FortController = Cast<AFortPlayerController>(Controller);
        if (FortController && FortController->InputYawScale != 0.0f)
        {
            Value /= FortController->InputYawScale;
        }
    }
    LocalSpin = Value;
    NativeRecoveredBool_0x63c = bFallingOnly;
}
