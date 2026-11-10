// CONFIDENCE: HIGH
// Original: exec RPC wrapper 0x142a4d4e0. Validate slot -> 0x1406801d0. Implementation slot -> 0x1423ac7a0.
// Read order: MarkActor (object, offset 0), then MarkText (FString, offset 8).
if (!ServerSetMarkText_Validate(MarkActor, MarkText))
{
	RPC_ValidateFailed(TEXT("ServerSetMarkText_Validate"));
	return;
}
ServerSetMarkText_Implementation(MarkActor, MarkText);
