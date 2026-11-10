// CONFIDENCE: HIGH
// Original: exec RPC wrapper 0x142a4d6f0. Validate slot -> 0x1406801d0. Implementation slot -> 0x1423acae0.
// Read order: bool, byte (offset 1), bool. Bool names follow inventory order; bool-to-bit mapping is MEDIUM.
if (!ServerSetReadyToContinue_Validate(bTryStartTimer, RequestedState, bReady))
{
	RPC_ValidateFailed(TEXT("ServerSetReadyToContinue_Validate"));
	return;
}
ServerSetReadyToContinue_Implementation(bTryStartTimer, RequestedState, bReady);
