// CONFIDENCE: MEDIUM. Fast-forwards toward FastForwardTimeOfDay over FastForwardDuration seconds. The speed is the hours to advance (wrapped by 24 when the target is behind) converted to seconds and divided by the duration and by the default speed. A non-positive duration is rejected with a warning, and nothing happens when the target equals the current time.
// Original addresses: exec 0x142b0ec60 (thunk). Body 0x141f5d0c0 (captured; the RestoreSpeed argument passes through to the internal setter).

if (FastForwardDuration <= 0.0f)
{
    UE_LOG(LogFortTimeOfDay, Warning, TEXT("AFortTimeOfDayManager::FastForwardToTime_OverTime() called with duration of 0.0 or less"));
    return;
}

float HoursToAdvance = FastForwardTimeOfDay - TimeOfDay;
if (HoursToAdvance == 0.0f)
{
    return;
}
if (HoursToAdvance < 0.0f)
{
    HoursToAdvance += 24.0f;
}

const float FastForwardSpeed = (HoursToAdvance * 3600.0f) / (FastForwardDuration * DefaultTimeOfDaySpeed);
FastForwardToTime(RestoreSpeed, FastForwardSpeed, FastForwardTimeOfDay);
