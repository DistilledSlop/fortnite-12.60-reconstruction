// CONFIDENCE: HIGH. Sets the time of day when it lies in [0, 24). Updates the replicated copy, applies the change through the day-night refresh, and logs at very verbose. Out-of-range values are ignored. The thunk does not return the result.
// Original addresses: exec 0x142b0ffa0 (thunk). Body 0x141f6eda0 (captured and read in full). Helper 0x141f798b0 (ApplyTimeOfDayChange).

if (TimeOfDayInHours < 0.0f || TimeOfDayInHours >= 24.0f)
{
    return;
}

UE_LOG(LogFortTimeOfDay, VeryVerbose, TEXT("SetTimeOfDayInternal(%p) NewTimeOfDay(%f)"), this, TimeOfDayInHours);
TimeOfDay = TimeOfDayInHours;
TimeOfDayReplicated = TimeOfDayInHours;
ApplyTimeOfDayChange(0.0f);
