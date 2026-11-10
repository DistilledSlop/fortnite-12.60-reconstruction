// CONFIDENCE: MEDIUM. Original exec 0x1427c2860; native body 0x1416a2bc0 (inlined).
// Thunk reads LaunchVelocity (FVector), bXYOverride, bZOverride in declaration order, then broadcasts them to the
// bound launch handlers in the multicast list at +0x980 (count +0x988, re-entrancy counter +0x994).
// Handlers are visited last-to-first. Each handler is invoked through vtable slot 0x50 with (PlayerBotPawn, &LaunchVelocity,
// bXYOverride, bZOverride); a false return or an invalid entry marks the list for compaction.
if (!PlayerBotPawn)
{
    return;
}

++NativeRecoveredInt32_0x994;

bool bNeedsCompaction = false;
for (int32 Index = NativeRecoveredInt32_0x988 - 1; Index >= 0; --Index)
{
    FNativeLaunchHandlerEntry& Entry = NativeRecoveredMulticastDelegate_0x980[Index];
    if (Entry.bIsValid == 0 || Entry.Handler == nullptr)
    {
        bNeedsCompaction = true;
        continue;
    }

    const bool bHandled = Entry.Handler->NativeOnLaunchedHandler(PlayerBotPawn, LaunchVelocity, bXYOverride, bZOverride);
    if (!bHandled)
    {
        bNeedsCompaction = true;
    }
}

--NativeRecoveredInt32_0x994;

if (bNeedsCompaction)
{
    NativeRecoveredMulticastDelegate_0x980.CompactInvocationList(false);
}
