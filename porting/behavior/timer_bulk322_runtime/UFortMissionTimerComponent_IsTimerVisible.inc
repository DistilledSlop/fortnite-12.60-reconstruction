// CONFIDENCE: high; captured native flow and typed dependency behavior.
{
    bool bHideTimer = false;
    const float Remaining = GetMissionTimerTimeRemaining();
    for (int32 Index = DisplayData.Num() - 1; Index >= 0; --Index)
    {
        if (DisplayData[Index].LessThanTimeValue <= Remaining)
        {
            break;
        }
        bHideTimer = DisplayData[Index].bHideTimer;
    }
    if (TimerVisibilityOverrideSetting == ETimerOverrideSetting::ForceHide)
    {
        bHideTimer = true;
    }
    else if (TimerVisibilityOverrideSetting == ETimerOverrideSetting::ShowAtEnd)
    {
        return Remaining <= TimerVisibilityShowAtEndTime;
    }
    else if (TimerVisibilityOverrideSetting != ETimerOverrideSetting::DefaultBehavior)
    {
        return true;
    }
    return !bHideTimer;
}
