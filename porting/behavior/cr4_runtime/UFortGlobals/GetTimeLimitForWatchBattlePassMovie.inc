// Behaviour snippet: UFortGlobals::GetTimeLimitForWatchBattlePassMovie (body only; spliced into the generated member function).
// Original address: 0x141ed7ae0 (thunk 0x142934310)
// Evidence: if the enum byte in CL is 0 (ETimeLimitForReplayCinematic::OpenTimeLimit) returns float +0x29c (OpenTimeLimitForWatchBattlePassMovie) of NativeGetGlobals() (0x141ed2350); otherwise returns 0.0f.
// CONFIDENCE: HIGH
static_assert(sizeof(UFortGlobals) > 0, "Generated header include for UFortGlobals.");
if (TimeLimitForWatchBattlePassMovie == ETimeLimitForReplayCinematic::OpenTimeLimit)
{
    return UFortGlobals::NativeGetGlobals()->OpenTimeLimitForWatchBattlePassMovie;
}
return 0.0f;
