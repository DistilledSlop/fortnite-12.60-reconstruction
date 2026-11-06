// CONFIDENCE: high
// Native target 0x142321d00.
const UFortHealthSet* FortHealthSet = HealthSet;
if (!FortHealthSet)
{
    return 0.0f;
}
return FortHealthSet->Shield.GetCurrentValue();
