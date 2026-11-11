// CONFIDENCE: HIGH. Sets the time-of-day speed when it is not negative. Forces a net update, copies the current time of day into the replicated copy, stores the speed, and logs at very verbose. Negative speeds are ignored.
// Original addresses: exec 0x142b101b0 (thunk). Body 0x141f6f180 (captured and read in full), which is SetTimeOfDaySpeedInternal.

if (InTimeOfDaySpeed < 0.0f)
{
    return;
}

ForceNetUpdate();
TimeOfDayReplicated = TimeOfDay;
TimeOfDaySpeed = InTimeOfDaySpeed;
UE_LOG(LogFortTimeOfDay, VeryVerbose, TEXT("SetTimeOfDaySpeedInternal(%p) NewTimeOfDay(%f) TimeOfDaySpeed(%f)"), this, TimeOfDay, TimeOfDaySpeed);
