// CONFIDENCE: HIGH
// Original: exec RPC wrapper 0x142a4e290. Validate slot -> 0x1423ad5a0. Implementation slot -> 0x1423ad4a0.
// Read order: object (offset 0), FText GameName (offset 8), FText array (offset 32).
if (!ServerUpdateGameDescriptionData_Validate(DataTarget, GameName, GameDescriptionText))
{
	RPC_ValidateFailed(TEXT("ServerUpdateGameDescriptionData_Validate"));
	return;
}
ServerUpdateGameDescriptionData_Implementation(DataTarget, GameName, GameDescriptionText);
