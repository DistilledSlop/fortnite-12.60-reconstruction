// CONFIDENCE: high
// Native target 0x142323de0.
const UFortHealthSet* FortHealthSet = HealthSet;
if (!FortHealthSet)
{
    return 0.0f;
}
return FortHealthSet->SignalInStorm.GetCurrentValue();
