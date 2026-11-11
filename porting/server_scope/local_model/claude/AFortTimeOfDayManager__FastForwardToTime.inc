// CONFIDENCE: MEDIUM. Starts a fast-forward of the time of day toward FastForwardTimeOfDay at FastForwardSpeed, a multiple of DefaultTimeOfDaySpeed. RestoreSpeed is the speed to return to afterwards; zero or below means the current speed. The restore value is stored as a scale of DefaultTimeOfDaySpeed. Records whether the target lies ahead of the current time.
// Original addresses: exec 0x142b0eb50 (thunk). Body inline in 0x141f5d050 (captured and read). Speed setter 0x141f6f180 (SetTimeOfDaySpeedInternal).

FastForwardTargetTimeOfDay = FastForwardTimeOfDay;

if (FMath::Abs(DefaultTimeOfDaySpeed) > 1e-8f)
{
    FastForwardRestoreSpeedScale = (RestoreSpeed <= 0.0f)
        ? (TimeOfDaySpeed / DefaultTimeOfDaySpeed)
        : (RestoreSpeed / DefaultTimeOfDaySpeed);
}

bFastForwardTowardLaterTime = (TimeOfDay <= FastForwardTimeOfDay) && (FastForwardTimeOfDay != TimeOfDay);

SetTimeOfDaySpeedInternal(DefaultTimeOfDaySpeed * FastForwardSpeed);
