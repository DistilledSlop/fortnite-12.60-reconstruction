// CONFIDENCE: high; captured native flow and typed dependency behavior.
{
    FMissionTimeDisplayData Result;
    Result.LessThanTimeValue = 1.0f;
    Result.bHideTimer = false;
    Result.BaseColor = FLinearColor::White;
    Result.PulseColor = FLinearColor::White;
    Result.ColorPulsesPerSecond = 1.0f;
    const float Remaining = GetMissionTimerTimeRemaining();
    for (int32 Index = DisplayData.Num() - 1; Index >= 0; --Index)
    {
        if (DisplayData[Index].LessThanTimeValue <= Remaining)
        {
            break;
        }
        Result = DisplayData[Index];
    }
    if (TimerVisibilityOverrideSetting == ETimerOverrideSetting::ForceHide)
    {
        Result.bHideTimer = true;
    }
    else if (TimerVisibilityOverrideSetting == ETimerOverrideSetting::ShowAtEnd)
    {
        Result.bHideTimer = TimerVisibilityShowAtEndTime < Remaining;
    }
    else if (TimerVisibilityOverrideSetting != ETimerOverrideSetting::DefaultBehavior)
    {
        Result.bHideTimer = false;
    }
    return Result;
}
