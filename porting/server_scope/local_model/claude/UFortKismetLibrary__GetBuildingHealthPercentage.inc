// CONFIDENCE: MEDIUM
// Native 0x14296c3f0 -> 0x141ef0ea0 -> 0x141b9ef10: the building's health percentage, current health over max health
// clamped to [0, 1]; 0 when there is no building or its max health is not positive.
if (!BuildingActor)
{
    return 0.0f;
}
return BuildingActor->GetHealthPercentage();
