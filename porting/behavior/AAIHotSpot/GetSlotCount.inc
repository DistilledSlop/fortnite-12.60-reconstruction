// Original address: 0x142710b80 (exec thunk; reads the EAIHotSpotSlotFilter byte, returns int32 from the helper below).
// Original body: 0x141629520 (native107-impl). Body-only; the generated thunk supplies the Filter argument.
// Helper traced from the cache and checked against the runtime image: the predicate 0x141291580 returns
//   UAIHotSpotSlot::SlotState (byte at +0x84), so the free count tests SlotState == EAIHotSpotSlot::Free (0).
//   The gate 0x141629520 tests bIsEnabled (AAIHotSpot, +0x264 mask 0x40), as the reflected bool bIsEnabled does.
// Filter values (EAIHotSpotSlotFilter): 0 All, 1 AvailableOnly, 2 UnavailableOnly.
// All returns Slots.Num(). AvailableOnly and UnavailableOnly count Free slots only when the hotspot is enabled and
//   Slots.Num() > 0 (count 0 otherwise); UnavailableOnly returns Num - count. Other values return 0.
// The helper dereferences each slot without a null guard; the port keeps that absence of guard (shipping code).
// CONFIDENCE: HIGH
if (Filter == 0)
{
	return Slots.Num();
}
int32 FreeCount = 0;
if (Filter != 0 && bIsEnabled)
{
	for (int32 Index = 0; Index < Slots.Num(); ++Index)
	{
		if (Slots[Index]->SlotState == EAIHotSpotSlot::Free)
		{
			++FreeCount;
		}
	}
}
if (Filter == 1)
{
	return FreeCount;
}
if (Filter == 2)
{
	return Slots.Num() - FreeCount;
}
return 0;
