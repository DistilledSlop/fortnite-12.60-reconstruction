// Original body 0x145127e40 (thunk 0x1452ec900): the button (RDX) is kept only when it IsA UFortSquadSlotSelectorButton (inline class-chain check against StaticClass, callee 0x1452e95c0). It is compared as a weak pointer against the TArray<TWeakObjectPtr> at +0x318 (Num at +0x320) with ported helper 0x1407327b0. The found index or -1 is stored to IndexOfSelectedSquadSlot (+0x29c) and broadcast on OnDifferentSquadSlotSelectedEvent (+0x268). ButtonIndex (R8) is not read by the body.
// CONFIDENCE: MEDIUM
static_assert(sizeof(UFortSquadSlotSelectorButton) > 0, "UFortSquadSlotSelectorButton supplies StaticClass");
static_assert(sizeof(UCreativeQuestComponent) > 0, "UCreativeQuestComponent declares the weak pointer equivalence helper");
UCommonButton* SelectedCandidate = SelectedButton;
if (SelectedCandidate == nullptr || !SelectedCandidate->IsA(UFortSquadSlotSelectorButton::StaticClass()))
{
    SelectedCandidate = nullptr;
}
TWeakObjectPtr<UObject> SelectedWeak = SelectedCandidate;
uint8* SquadSlotsRaw = reinterpret_cast<uint8*>(this);
TArray<TWeakObjectPtr<UObject>>& SlotWeakPtrs = *reinterpret_cast<TArray<TWeakObjectPtr<UObject>>*>(SquadSlotsRaw + 0x318);
int32 FoundSlotIndex = -1;
for (int32 SlotIndex = 0; SlotIndex < SlotWeakPtrs.Num(); ++SlotIndex)
{
    if (UCreativeQuestComponent::NativeWeakObjectPtrEquivalent1407327b0(reinterpret_cast<const uint8*>(&SlotWeakPtrs[SlotIndex]), reinterpret_cast<const uint8*>(&SelectedWeak)))
    {
        FoundSlotIndex = SlotIndex;
        break;
    }
}
IndexOfSelectedSquadSlot = FoundSlotIndex;
OnDifferentSquadSlotSelectedEvent.Broadcast(FoundSlotIndex);
