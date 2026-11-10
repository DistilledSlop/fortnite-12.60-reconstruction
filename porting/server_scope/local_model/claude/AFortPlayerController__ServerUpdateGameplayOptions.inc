// CONFIDENCE: HIGH
// Original: exec RPC wrapper 0x142a4e470. Validate slot -> 0x1423aafe0. Implementation slot -> 0x1423ad610.
// Read order: keys array (offset 0), values array (offset 16).
if (!ServerUpdateGameplayOptions_Validate(UserOptionsKeys, UserOptionsValues))
{
	RPC_ValidateFailed(TEXT("ServerUpdateGameplayOptions_Validate"));
	return;
}
ServerUpdateGameplayOptions_Implementation(UserOptionsKeys, UserOptionsValues);
