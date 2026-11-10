// CONFIDENCE: HIGH
// Original: exec RPC wrapper 0x142a4dff0. Validate slot -> 0x1406801d0. Implementation slot -> 0x1423ad150.
// Read order: int at offset 0 (ChosenItem), then int at offset 4 (MenuIdentifier). Inventory lists them in reverse; offsets decide the order (MEDIUM).
if (!ServerUIChoiceCompleted_Validate(ChosenItem, MenuIdentifier))
{
	RPC_ValidateFailed(TEXT("ServerUIChoiceCompleted_Validate"));
	return;
}
ServerUIChoiceCompleted_Implementation(ChosenItem, MenuIdentifier);
