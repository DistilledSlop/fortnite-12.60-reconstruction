// CONFIDENCE: high; captured native flow and typed dependency behavior.
{
    const AActor* Owner = GetOwner();
    if (Owner && Owner->GetLocalRole() == ROLE_Authority)
    {
        if (!NativeRecoveredTimerDelegate.IsBound())
        {
            return -1.0f;
        }
        if (TimerData.bTimerIsPaused)
        {
            return TimerData.ReplicatedRemainingTime;
        }
        const float Remaining = Owner->GetWorldTimerManager().GetTimerRemaining(NativeRecoveredTimerHandle);
        if (Remaining < 0.0f) return 0.0f;
        return Remaining >= MAX_flt ? MAX_flt : Remaining;
    }
    if (TimerData.bTimerIsPaused)
    {
        return TimerData.ReplicatedRemainingTime;
    }
    if (!(TimerData.ClientRemainingTime >= 0.0f))
    {
        return -1.0f;
    }
    const float Remaining = TimerData.ClientRemainingTime - GetWorld()->GetTimeSeconds();
    if (!(Remaining >= 0.0f)) return 0.0f;
    return Remaining >= MAX_flt ? MAX_flt : Remaining;
}
