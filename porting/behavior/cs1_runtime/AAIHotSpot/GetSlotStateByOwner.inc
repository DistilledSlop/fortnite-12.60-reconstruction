// Original body address: 0x1416297a0 (thunk 0x142711070)
// Offset evidence: Slots is the reflected TArray at this+0x238 (Num at +0x240). A missing or out-of-range index, or a null slot, returns 4. Otherwise the helper NativeHelper141291580 is tail-called with the slot pointer.
// The Slots element type is UAIHotSpotSlot*; the helper receives the same pointer as AAIHotSpot*.
// CONFIDENCE: HIGH
const int32 SlotIndex = GetSlotIndexByOwner(AIOwner);
if (SlotIndex < 0 || SlotIndex >= Slots.Num())
{
    return static_cast<EAIHotSpotSlot>(4);
}
UAIHotSpotSlot* const SlotObject = Slots[SlotIndex];
if (SlotObject == nullptr)
{
    return static_cast<EAIHotSpotSlot>(4);
}
return static_cast<EAIHotSpotSlot>(NativeHelper141291580(reinterpret_cast<AAIHotSpot*>(SlotObject)));
