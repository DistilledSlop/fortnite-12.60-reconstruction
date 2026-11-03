// CONFIDENCE: high; captured native flow and typed dependency behavior.
{
    float Elapsed = -1.0f;
    const AActor* Owner = GetOwner();
    if (Owner && Owner->GetLocalRole() == ROLE_Authority)
    {
        if (NativeRecoveredTimerDelegate.IsBound())
        {
            const float Remaining = TimerData.bTimerIsPaused
                ? TimerData.ReplicatedRemainingTime
                : Owner->GetWorldTimerManager().GetTimerRemaining(NativeRecoveredTimerHandle);
            Elapsed = TimerData.OriginalTimePeriod - Remaining;
        }
    }
    else if (TimerData.bTimerIsPaused)
    {
        Elapsed = TimerData.OriginalTimePeriod - TimerData.ReplicatedRemainingTime;
    }
    else if (!(TimerData.ClientRemainingTime < 0.0f))
    {
        Elapsed = TimerData.OriginalTimePeriod - (TimerData.ClientRemainingTime - GetWorld()->GetTimeSeconds());
    }
    Elapsed += TimerData.TimeAddedOrRemoved;
    if (Elapsed < 0.0f) return 0.0f;
    return Elapsed < MAX_flt ? Elapsed : MAX_flt;
}
