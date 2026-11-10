// CONFIDENCE: MEDIUM. Sets bUsesDayPhaseChange and registers or unregisters this building with the day-phase subsystem to match. Registration requires the native override NativeUsesDayPhaseChange() to return true.
// Original address: 0x142738680 (exec). Native body: 0x141bc5220. The decompiled changed-check compares against a sign-extended bit; the body here uses a plain changed check.
if (bUsesDayPhaseChange == bUseDayPhase)
{
    return;
}

bUsesDayPhaseChange = bUseDayPhase;

if (bUseDayPhase)
{
    if (bDayPhaseRegistrationPending && NativeUsesDayPhaseChange() && !bRegisteredForDayPhaseChange)
    {
        RegisterForDayPhaseChange();
    }
}
else if (bRegisteredForDayPhaseChange)
{
    UnregisterFromDayPhaseChange();
}
