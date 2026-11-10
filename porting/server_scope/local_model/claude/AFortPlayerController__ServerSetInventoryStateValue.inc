// CONFIDENCE: HIGH
// Original: exec RPC wrapper 0x142a4d3a0. Validate slot -> 0x1406801d0. Implementation slot -> 0x1423ac760.
// Read order: ItemGuid (offset 0), then StateValue (offset 16). Struct type names unresolved; both are 16-byte values.
if (!ServerSetInventoryStateValue_Validate(ItemGuid, StateValue))
{
	RPC_ValidateFailed(TEXT("ServerSetInventoryStateValue_Validate"));
	return;
}
ServerSetInventoryStateValue_Implementation(ItemGuid, StateValue);
