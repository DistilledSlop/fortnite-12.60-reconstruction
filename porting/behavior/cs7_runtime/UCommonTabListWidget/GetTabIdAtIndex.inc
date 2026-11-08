// Original address: 0x1407c5370 (thunk 0x140815280)
// Evidence: [this+0x2b8] = RegisteredTabsByID (TMap<FName, FCommonRegisteredTabInfo>). Early exit `Index >= [+0x2c0] - [+0x2ec]` is TMap::Num() (Pairs.Num - NumFreeIndices).
// Element scan compares element+8 (FCommonRegisteredTabInfo::TabIndex at value offset 0) with Index and returns the key qword at element+0 (FName) on match, else NAME_None.
// The set-bit iterator advance (0x140687d00) is the range-for step over the TMap and is not reproduced.
// CONFIDENCE: MEDIUM
if (Index >= RegisteredTabsByID.Num())
{
	return NAME_None;
}
for (const auto& Pair : RegisteredTabsByID)
{
	if (Pair.Value.TabIndex == Index)
	{
		return Pair.Key;
	}
}
return NAME_None;
