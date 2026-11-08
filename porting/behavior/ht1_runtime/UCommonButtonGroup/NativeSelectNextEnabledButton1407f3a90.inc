// Helper 0x1407f3a90 (UCommonButtonGroup). Tail-jumped from SelectNextButton @0x1407f3a80: RCX=this, EDX=StartIndex (caller loads [this+0xc0]), R8B=bWrapAround.
// Offset evidence: [this+0xb8] int32 slot count, [this+0xb0] TArray data pointer of 8-byte TWeakObjectPtr<UCommonButton> entries. No reflected member covers 0xb0/0xb8, so raw access is used.
// Calls: 0x142ee5dd0 TWeakObjectPtr::Get, 0x1407cf620 UCommonButton::IsInteractionEnabled (bool), 0x1407f39f0 UCommonButtonGroup::SelectButtonAtIndex(int32).
// All 40 asm lines accounted for: scan forward from StartIndex; at the last slot wrap to 0 only if bWrapAround; skip null or non-interactable slots; select the first valid one.
// CONFIDENCE: HIGH

int32 CandidateIdx = StartIndex;
for (;;)
{
    const int32 SlotCount = *reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(ButtonGroupObj) + 0xb8);
    if (CandidateIdx >= SlotCount - 1)
    {
        if (!bWrapAround)
        {
            return;
        }
        CandidateIdx = 0;
    }
    else
    {
        CandidateIdx += 1;
        if (CandidateIdx < 0)
        {
            return;
        }
    }
    if (CandidateIdx >= SlotCount)
    {
        return;
    }
    if (CandidateIdx < 0)
    {
        continue;
    }
    TWeakObjectPtr<UCommonButton>* ButtonSlots = *reinterpret_cast<TWeakObjectPtr<UCommonButton>* const*>(reinterpret_cast<const uint8*>(ButtonGroupObj) + 0xb0);
    UCommonButton* CandidateButton = ButtonSlots[CandidateIdx].Get();
    if (CandidateButton == nullptr || !CandidateButton->IsInteractionEnabled())
    {
        continue;
    }
    ButtonGroupObj->SelectButtonAtIndex(CandidateIdx);
    return;
}
