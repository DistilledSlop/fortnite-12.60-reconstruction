// CONFIDENCE: HIGH
// Original: exec 0x142a46a40 (no thunk; native body only). Interaction component at AFortPlayerController+0x2010.
// The delegate invocation list is at component+0xc0 (TArray, 16-byte entries: UObject* Object, FName FunctionName),
// the element count at +0xc8, a re-entrancy counter at +0xd4, and a blocker pointer at +0x180.
UFortControllerComponent_Interaction* Interaction = InteractionComponent;
if (!Interaction || Interaction->FixupBlocker != nullptr)
{
    return;
}

++Interaction->FixupLockCount;

bool bNeedsCompaction = false;
for (int32 Index = Interaction->FixupInvocationList.Num() - 1; Index >= 0; --Index)
{
    const FFixupInvocationEntry& Entry = Interaction->FixupInvocationList[Index];
    if (Entry.FunctionName.GetComparisonIndex() == 0 || Entry.Object == nullptr)
    {
        bNeedsCompaction = true;
    }
    else if (!Entry.Object->NativeIsInvocationTargetValid()) // vslot 0x50 on UObject; base body 0x1406801e0 returns false
    {
        bNeedsCompaction = true;
    }
}

--Interaction->FixupLockCount;

if (bNeedsCompaction)
{
    FMulticastDelegateBase_CompactInvocationList(Interaction->FixupInvocationList, false);
}
