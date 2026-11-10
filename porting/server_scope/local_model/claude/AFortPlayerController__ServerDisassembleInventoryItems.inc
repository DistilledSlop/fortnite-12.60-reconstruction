// CONFIDENCE: MEDIUM. Original exec 0x142a4ae80 (RPC thunk). Parameters: ItemGuids (TArray<FGuid>), ItemCounts (TArray<int32>), in declaration order.
// Validate is virtual slot 0x1030 -> 0x1423aafe0: returns ItemGuids.Num() == ItemCounts.Num(). Ported below.
// Implementation is virtual slot 0x1038 -> 0x1423aaec0 (68 lines). It packs the two arrays into one array of {FGuid, int32} pairs (element stride 0x14) and hands it to the disassemble helper 0x142376c20 (323 lines, not ported).
if (ItemGuids.Num() != ItemCounts.Num())
{
	RPC_ValidateFailed(TEXT("ServerDisassembleInventoryItems_Validate"));
	return;
}
TArray<FDisassembleEntry> Entries;
for (int32 Index = 0; Index < ItemGuids.Num(); ++Index)
{
	FDisassembleEntry Entry;
	Entry.ItemGuid = ItemGuids[Index];
	Entry.Count = ItemCounts[Index];
	Entries.Add(Entry);
}
DisassembleInventoryItems_0x142376c20(this, Entries);
