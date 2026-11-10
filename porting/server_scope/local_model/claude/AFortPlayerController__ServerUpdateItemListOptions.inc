// CONFIDENCE: HIGH
// Original: exec RPC wrapper 0x142a4e5d0. Validate slot -> 0x1406801d0. Implementation slot -> 0x1423add90.
// Read order: object (offset 0), int (offset 8), keys array (offset 16), values array (offset 32).
if (!ServerUpdateItemListOptions_Validate(ItemContainerComponent, ItemIndex, OptionsKeys, OptionsValues))
{
	RPC_ValidateFailed(TEXT("ServerUpdateItemListOptions_Validate"));
	return;
}
ServerUpdateItemListOptions_Implementation(ItemContainerComponent, ItemIndex, OptionsKeys, OptionsValues);
