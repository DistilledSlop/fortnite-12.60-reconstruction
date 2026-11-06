// CONFIDENCE: high; native timer selection, float tick rounding, and printf formats recovered.
{
    float Seconds;
    if (bShowTimeElapsed)
    {
        Seconds = -1.0f;
        const AActor* Owner = GetOwner();
        if (Owner && Owner->GetLocalRole() == ROLE_Authority)
        {
            if (NativeRecoveredTimerDelegate.IsBound())
            {
                const float Remaining = TimerData.bTimerIsPaused
                    ? TimerData.ReplicatedRemainingTime
                    : Owner->GetWorldTimerManager().GetTimerRemaining(NativeRecoveredTimerHandle);
                Seconds = TimerData.OriginalTimePeriod - Remaining;
            }
        }
        else if (TimerData.bTimerIsPaused)
        {
            Seconds = TimerData.OriginalTimePeriod - TimerData.ReplicatedRemainingTime;
        }
        else if (TimerData.ClientRemainingTime >= 0.0f)
        {
            Seconds = TimerData.OriginalTimePeriod - (TimerData.ClientRemainingTime - GetWorld()->GetTimeSeconds());
        }
        Seconds += TimerData.TimeAddedOrRemoved;
    }
    else
    {
        Seconds = GetMissionTimerTimeRemaining();
    }
    if (!(Seconds >= 0.0f)) Seconds = 0.0f;
    else if (Seconds >= MAX_flt) Seconds = MAX_flt;

    // The original performs these operations in single precision, then floors the result.
    const float RoundedTicks = Seconds * 10000000.0f + 0.5f;
    const double TickValue = static_cast<double>(RoundedTicks);
    // CVTTSD2SI returns INT64_MIN for overflow, infinity, or NaN on the original platform.
    const int64 Ticks = (TickValue >= 9223372036854775808.0 || !(TickValue >= -9223372036854775808.0))
        ? MIN_int64 : static_cast<int64>(FMath::FloorToDouble(TickValue));
    const int32 Minutes = static_cast<int32>((Ticks / 600000000) % 60);
    const int32 WholeSeconds = static_cast<int32>((Ticks / 10000000) % 60);
    if (static_cast<double>(Ticks) * (1.0 / 36000000000.0) < 1.0)
    {
        return FText::FromString(FString::Printf(TEXT("%1d:%02d"), Minutes, WholeSeconds));
    }
    const int32 Hours = static_cast<int32>((Ticks / 36000000000LL) % 24);
    return FText::FromString(FString::Printf(TEXT("%1d:%02d:%02d"), Hours, Minutes, WholeSeconds));
}
