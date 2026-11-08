// Behaviour snippet: UFortGlobals::GetTimeLimitForReplayCinematic (body only; spliced into the generated member function).
// Original address: 0x141ed7a80 (thunk 0x142934290)
// Evidence: switch on the enum byte in CL: 0 -> float +0x288 (OpenTimeLimitForReplayCinematic); 1 -> float +0x28c (DurationTimeLimitForReplayCinematic); 2 -> float +0x290 (DurationExtraTimeForReplayCinematic). Any other value returns 0.0f without calling NativeGetGlobals (0x141ed2350). Enumerator values 0/1/2 from reflection enum ETimeLimitForReplayCinematic.
// CONFIDENCE: HIGH
static_assert(sizeof(UFortGlobals) > 0, "Generated header include for UFortGlobals.");
switch (TimeLimitForReplayCinematic)
{
case ETimeLimitForReplayCinematic::OpenTimeLimit:
    return UFortGlobals::NativeGetGlobals()->OpenTimeLimitForReplayCinematic;
case ETimeLimitForReplayCinematic::DurationTimeLimit:
    return UFortGlobals::NativeGetGlobals()->DurationTimeLimitForReplayCinematic;
case ETimeLimitForReplayCinematic::DurationExtraTime:
    return UFortGlobals::NativeGetGlobals()->DurationExtraTimeForReplayCinematic;
default:
    return 0.0f;
}
