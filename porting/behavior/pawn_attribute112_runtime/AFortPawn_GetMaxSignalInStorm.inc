// CONFIDENCE: high
// Native target 0x142321d20.
const UFortHealthSet* FortHealthSet = HealthSet;
if (!FortHealthSet)
{
    return 0.0f;
}
return FortHealthSet->MaxSignalInStorm.GetCurrentValue();
