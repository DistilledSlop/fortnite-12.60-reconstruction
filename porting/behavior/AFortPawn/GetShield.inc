// CONFIDENCE: high
// Native target 0x142323a70.
const UFortHealthSet* FortHealthSet = HealthSet;
if (!FortHealthSet)
{
    return 0.0f;
}
return FortHealthSet->CurrentShield.GetCurrentValue();
