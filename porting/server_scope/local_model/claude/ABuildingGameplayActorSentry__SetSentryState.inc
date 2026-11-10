// CONFIDENCE: HIGH. Clears the pending sentry timer, stores the new sentry state and runs the replicated OnRep on the server.
// Original address: 0x1427445b0 (exec). Native body: 0x141beb180.
if (UWorld* World = GetWorld())
{
    // Timer manager is picked through the world's owner chain in the native body (MEDIUM detail).
    FTimerManager& TimerManager = World->GetTimerManager();
    if (TimerManager.FindTimer(SentryTimerHandle) != nullptr)   // FTimerHandle member at +0x888 (name not recovered)
    {
        TimerManager.ClearTimer(SentryTimerHandle);
    }
    SentryTimerHandle.Invalidate();
}
CurrentSentryState = InSentryState;   // byte at +0x7b3, replicated with OnRep_CurrentSentryState
OnRep_CurrentSentryState();           // ProcessEvent of the rep notify (native helper 0x1427428d0)
