// CONFIDENCE: high
// Native target 0x14231c4c0.
if (GetTearOff())
{
    return 0.0f;
}
if (!bHealthSynced || !HealthSet)
{
    return 1.0f;
}
const UFortHealthSet* FortHealthSet = HealthSet;
const float HealthValue = FortHealthSet->Health.GetCurrentValue();
return (HealthValue <= 0.0f) ? 0.0f : HealthValue;
