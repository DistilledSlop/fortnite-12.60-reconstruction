// Draft of 0x14205c5e0 and 0x14205c7f0. Integration needs non-reflected
// AFortQuickBars members: a TMap<uint8, FTimerHandle> at native +0x480,
// a compatible multicast at +0x3d0, and the callback declaration. The map miss path 0x141b31980 inserts a
// byte key and a zero-initialized 8-byte handle.
// NativeUpdateQuickBar142063590 and NativeBroadcastQuickBarDirtySlots are
// unresolved bindings; see evidence.json for the captured refresh dependencies.
void AFortQuickBars::NativeScheduleQuickBarUpdate(uint8 Bar)
{
    if (!Cast<AFortPlayerController>(GetOwner()))
        return;

    FTimerHandle& Handle = NativeQuickBarTimerHandles.FindOrAdd(Bar); // 0x142040380
    if (Handle.IsValid())
        return;

    UWorld* World = GetWorld();
    if (!World)
        return;

    FTimerDelegate Delegate = FTimerDelegate::CreateUObject(
        this, &AFortQuickBars::NativeFlushQuickBarUpdate, Bar);
    Handle = World->GetTimerManager().SetTimerForNextTick(Delegate); // 0x144627d90
}

void AFortQuickBars::NativeFlushQuickBarUpdate(uint8 Bar) // 0x14205c7f0
{
    NativeQuickBarTimerHandles.FindOrAdd(Bar).Invalidate();
    FQuickBar& QuickBar = Bar == 1 ? SecondaryQuickBar :
                          Bar == 2 ? CreativeQuickBar : PrimaryQuickBar;
    TArray<int32> DirtySlots;
    for (int32 Index = 0; Index < QuickBar.Slots.Num(); ++Index)
    {
        FQuickBarSlot& Slot = QuickBar.Slots[Index];
        if (Slot.bIsDirty)
        {
            DirtySlots.Add(Index);
            Slot.bIsDirty = false;
        }
    }
    if (DirtySlots.Num() == 0)
        return;

    if (AFortPlayerController* Owner = Cast<AFortPlayerController>(GetOwner()))
        NativeUpdateQuickBar142063590(QuickBar, Owner);
    NativeBroadcastQuickBarDirtySlots(Bar, DirtySlots); // native multicast +0x3d0
}
