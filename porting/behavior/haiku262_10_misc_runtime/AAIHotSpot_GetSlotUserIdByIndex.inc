// Native0x1416297e0 CONFIDENCE: high
// Slots at +0x238/+0x240; UAIHotSpotSlot::UserId at +0x6c (layout offset 108). Invalid index or null slot returns -1 (0xffffffff).
if (Index < 0 || Slots.Num() <= Index)
{
    return -1;
}
UAIHotSpotSlot* Slot = Slots[Index];
if (Slot == nullptr)
{
    return -1;
}
return Slot->UserId;
