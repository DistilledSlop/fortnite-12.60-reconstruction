// Native0x1416296d0 CONFIDENCE: high
// Slots TArray data/ArrayNum at +0x238/+0x240 (layout Slots offset 568); UAIHotSpotSlot::Owner at +0x78 (layout offset 120).
if (Index < 0 || Slots.Num() <= Index)
{
    return nullptr;
}
UAIHotSpotSlot* Slot = Slots[Index];
if (Slot == nullptr)
{
    return nullptr;
}
return Slot->Owner;
