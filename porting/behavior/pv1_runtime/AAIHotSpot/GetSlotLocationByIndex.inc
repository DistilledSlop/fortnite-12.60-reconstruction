// Original body 0x141629660 (thunk 0x142710d40) for AAIHotSpot::GetSlotLocationByIndex.
// Offset evidence: [RCX+0x238] is AAIHotSpot::Slots (reflected TArray<UAIHotSpotSlot*>, Data at +0x238, Num at +0x240); sret FVector in RDX, Index in R8D.
// Fallback FVector is the .data snapshot 0x1478c3890/0x1478c3898 = 0x7f7fffff (MAX_flt) in all three components.
// CONFIDENCE: HIGH
if (Slots.IsValidIndex(Index))
{
	static_assert(sizeof(UAIHotSpotSlot) > 0, "UAIHotSpotSlot must be complete");
	UAIHotSpotSlot* Slot = Slots[Index];
	if (Slot != nullptr)
	{
		return Slot->GetSlotLocation();
	}
}
return FVector(MAX_flt, MAX_flt, MAX_flt);
