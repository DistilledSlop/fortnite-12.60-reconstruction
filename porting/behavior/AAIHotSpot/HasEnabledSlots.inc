// Original address: 0x142711280 (exec thunk; no arguments; writes the bool result of the helper below).
// Original body: 0x14162bd80 (native107-impl). The helper scans Slots from index 0 and returns true at the first
//   non-null slot whose bIsEnabled is set (UAIHotSpotSlot bIsEnabled, byte +0x70 mask 0x20, read by 0x141663760).
//   It returns false when no such slot exists, including an empty Slots array.
// CONFIDENCE: HIGH
for (int32 Index = 0; Index < Slots.Num(); ++Index)
{
	UAIHotSpotSlot* Slot = Slots[Index];
	if (Slot != nullptr && Slot->bIsEnabled)
	{
		return true;
	}
}
return false;
