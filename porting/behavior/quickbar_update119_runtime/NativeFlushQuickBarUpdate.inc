// CONFIDENCE: high
// Original 0x14205c7f0 (timer callback); Bar is the raw EFortQuickBars byte captured by the scheduler.
// The keyed handle is reset before anything else; a missing key is inserted first, as the native lookup does.
QuickBarUpdateTimersNative.FindOrAdd(Bar) = FTimerHandle();

// Bar 1 selects Secondary, 2 selects Creative, every other value (including 0) selects Primary.
FQuickBar& QuickBar = Bar == 1 ? SecondaryQuickBar : (Bar == 2 ? CreativeQuickBar : PrimaryQuickBar);

// Ascending scan: collect dirty indices and clear each dirty bit as it is collected.
TArray<int32> DirtySlots;
for (int32 Index = 0; Index < QuickBar.Slots.Num(); ++Index)
{
    if (QuickBar.Slots[Index].bIsDirty)
    {
        DirtySlots.Add(Index);
        QuickBar.Slots[Index].bIsDirty = false;
    }
}

if (DirtySlots.Num() == 0)
{
    return;
}

// Owner is rechecked here, after the scan; the refresh receives the bar and the controller.
if (AFortPlayerController* OwnerController = Cast<AFortPlayerController>(GetOwner()))
{
    QuickBar.RefreshEquippedItemDefinitions(OwnerController);
}

// Native multicast at +0x3d0: lock, reverse invocation order, ExecuteIfSafe, compaction of expired bindings.
// The bar byte is passed unmapped, exactly as captured.
QuickBarDirtySlotsNative.Broadcast(static_cast<EFortQuickBars>(Bar), DirtySlots);
