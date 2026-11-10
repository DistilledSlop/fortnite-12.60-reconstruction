// CONFIDENCE: LOW
// Original target 0x1419f0ea0. ABI: GUID (16-byte key) in the first slot, ItemWrap (soft object) in the second.
if (WorldInventory == nullptr)
{
	return;
}
if (ItemWrap.Field0 == 0 && ItemWrap.Field1 == 0)
{
	return;
}
// Look up the inventory item by GUID (vslot 0x18 on WorldInventory+0x218, a TMap-style find).
FItemKey ItemKey = GUID;
auto* Item = WorldInventory.Items.FindByKey(ItemKey);	// placeholder for vslot 0x18 lookup
if (Item != nullptr)
{
	// Skip when the item already carries the global gameplay-tag gate (FUN_141ed1800 global at +0x19f0).
	if (Item->TagContainer.HasTag(GlobalGate_0x19f0))
	{
		return;
	}
	// Skip when the item is tagged Weapon.Meta.DoNotApplyWraps (FUN_142cd4ab0 builds the tag from a literal).
	FGameplayTag DoNotApply = RequestGameplayTag(TEXT("Weapon.Meta.DoNotApplyWraps"));
	if (Item->TagContainer.HasTag(DoNotApply))
	{
		return;
	}
	// FUN_140d8f4d0: sub-object at item+0xe0; FUN_142038a40 stores the wrap there.
	if (auto* ItemState = Item->ItemStateSubobject_0xe0)
	{
		ItemState->SetOverrideWrap(ItemWrap);	// writes wrap at +0xc8 and copies the 16-byte tag block (FUN_142ebd9e0)
	}
}
// Pawn-side path: FUN_1419b6cd0 returns the controller's pawn when it is an AFortPlayerPawn.
if (auto* Pawn = GetFortPlayerPawnOrNull(this))
{
	// FUN_142316c40: find the pawn's item entry by GUID (array at +0x598, count at +0x5a0).
	if (auto* PawnItem = Pawn->FindInventoryEntryByGuid(GUID))
	{
		FWrapRecord Record = FromSoftObject(ItemWrap);	// copies fields from ItemWrap (FUN_1426b3510 consumes it)
		PawnItem->ApplyOverrideWrapRecord(Record);
	}
}
