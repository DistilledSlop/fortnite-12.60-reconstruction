// CONFIDENCE: MEDIUM
// Original target 0x1419f0c30. Appends {Count, ItemDefinition} to CachedPurchasedItems, then logs.
// Argument order follows the thunk ABI (object then int32).
int32 OldNum = CachedPurchasedItems.Num();
CachedPurchasedItems.SetNum(OldNum + 1);	// FUN_140690dd0 grows storage when ArrayNum exceeds ArrayMax
auto& Entry = CachedPurchasedItems[OldNum];	// element: int32 at +0, object at +8 (16 bytes)
Entry.Count = Count;
Entry.ItemDefinition = ItemDefinition;
if (ItemDefinition == nullptr)
{
	if (LogVerbosity > 2)
	{
		FMsg_Logf_Internal(LogCategory, 3, TEXT("Caching Purchased Items: None with Count: %d"), Count);
	}
}
else if (LogVerbosity > 2)
{
	ItemDefinition->VirtualSlot268();	// vslot 0x268 (GetName-like), result converted by FUN_142c40540
	FMsg_Logf_Internal(LogCategory, 3, TEXT("Caching Purchased Items: %s with count: %d"), ItemDefinitionName, Count);
}
// Trailing shared-reference release on ItemDefinition (FUN_... lock-decrement at +8) is a by-value copy artifact.
