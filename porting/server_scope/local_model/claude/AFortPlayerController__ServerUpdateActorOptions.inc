// CONFIDENCE: HIGH
// Original: exec RPC wrapper 0x142a4e0f0. Validate slot -> 0x1423ad490. Implementation slot -> 0x1423ad260.
// Read order: object (offset 0), keys array (offset 8), values array (offset 24).
if (!ServerUpdateActorOptions_Validate(OptionsTarget, OptionsKeys, OptionsValues))
{
	RPC_ValidateFailed(TEXT("ServerUpdateActorOptions_Validate"));
	return;
}
ServerUpdateActorOptions_Implementation(OptionsTarget, OptionsKeys, OptionsValues);
