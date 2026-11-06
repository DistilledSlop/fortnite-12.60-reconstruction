// CONFIDENCE: high
// Original 0x14205c5e0 (deferred quickbar update scheduler); Bar is the raw EFortQuickBars byte.
if (!Cast<AFortPlayerController>(GetOwner()))
{
    return;
}

// 0x142040380: find-or-add keyed by the byte, a miss inserts a zero handle (0x141b31980).
FTimerHandle& Handle = QuickBarUpdateTimersNative.FindOrAdd(Bar);
if (Handle.IsValid())
{
    return;
}

UWorld* World = GetWorld();
if (!World)
{
    return;
}

// Weak UObject delegate carrying the bar byte, 0x14205c7f0 as the method (0x144627d90 next-tick scheduling).
const FTimerDelegate Delegate = FTimerDelegate::CreateUObject(this, &AFortQuickBars::NativeFlushQuickBarUpdate, Bar);
Handle = World->GetTimerManager().SetTimerForNextTick(Delegate);
