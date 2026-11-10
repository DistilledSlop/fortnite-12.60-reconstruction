// CONFIDENCE: HIGH
// Original address: 0x142a5a860 (RPC thunk). FFrame read order: ItemDefinition (object, +0), Count (int, +8).
if (!ServerAddToCachedPurchased_Validate(ItemDefinition, Count))
{
	RPC_ValidateFailed(TEXT("ServerAddToCachedPurchased_Validate"));
	return;
}
ServerAddToCachedPurchased_Implementation(ItemDefinition, Count);
