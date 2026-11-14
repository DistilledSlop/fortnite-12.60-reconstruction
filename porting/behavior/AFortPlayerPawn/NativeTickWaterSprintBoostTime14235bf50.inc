// CONFIDENCE: HIGH. Original14235bf50; MAXSS chooses zero for an unordered subtraction.
if (NativeWaterSprintBoostTimeRemaining12d4 > 0.0f)
{
    const float Remaining = NativeWaterSprintBoostTimeRemaining12d4 - DeltaSeconds;
    NativeWaterSprintBoostTimeRemaining12d4 = Remaining > 0.0f ? Remaining : 0.0f;
    if (NativeWaterSprintBoostTimeRemaining12d4 == 0.0f)
    {
        NativeEndWaterSprintBoost142314570();
    }
}
