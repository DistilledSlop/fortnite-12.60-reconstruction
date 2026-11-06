// Native0x141629770 CONFIDENCE: high
// Slots at +0x238/+0x240; UAIHotSpotSlot::SlotState byte at +0x84 (layout offset 132). EAIHotSpotSlot::Disabled == 4 (header).
if (Index > -1 && Index < Slots.Num())
{
    UAIHotSpotSlot* Slot = Slots[Index];
    if (Slot != nullptr)
    {
        return Slot->SlotState;
    }
}
return EAIHotSpotSlot::Disabled;
