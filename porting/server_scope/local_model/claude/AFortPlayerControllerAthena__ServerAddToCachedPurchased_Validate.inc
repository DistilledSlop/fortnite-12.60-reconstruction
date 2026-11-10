// CONFIDENCE: LOW
// Original target 0x1419f0d80. Argument order follows the thunk ABI: ItemDefinition (object) then Count (int32).
// Structure of the original check (helper internals are placeholders, see evidence notes):
if (ItemDefinition == nullptr || Count <= 0)
{
	return false;
}
// PurchaseTable: FUN_141a78050(FUN_141a777c0()) returns the asset-manager table at +0x2f8.
auto PurchaseTable = Helper_141a78050(Helper_141a777c0());
// Need PurchaseTable.Num - PurchaseTable.NumUsed > 0 (fields at +0x300 and +0x32c of the manager).
if (!(PurchaseTable.Num() - PurchaseTable.NumUsed() > 0))
{
	return false;
}
// Key from ItemDefinition via vslot 0x190; FUN_1419d1360 returns the entry index (-1 when absent).
auto Key = ItemDefinition->VirtualKey190();
int32 EntryIndex = Helper_1419d1360(PurchaseTable, Key);
if (EntryIndex == -1)
{
	return false;
}
// Entry value is the int at entry+0x10 (element size 0x1c); compare Count against value * UFortRuntimeOptions +0x4fc.
int32 EntryValue = PurchaseTable.EntryAt(EntryIndex).Value;
int32 Multiplier = UFortRuntimeOptions_GetDefault()->Multiplier_0x4fc;
if (Count <= EntryValue * Multiplier && Helper_141a074e0(this))
{
	return true;
}
return false;
