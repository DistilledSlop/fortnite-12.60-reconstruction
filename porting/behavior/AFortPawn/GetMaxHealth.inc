// CONFIDENCE: high
// Native target 0x141948150.
const UFortHealthSet* FortHealthSet = HealthSet;
if (!FortHealthSet)
{
    return 0.0f;
}
return FortHealthSet->MaxHealth.GetCurrentValue();
