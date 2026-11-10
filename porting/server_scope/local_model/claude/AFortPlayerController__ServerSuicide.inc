// CONFIDENCE: HIGH
// Original: exec RPC wrapper 0x142a4dd00. Validate slot -> 0x1406801d0. Implementation slot -> 0x1423acfe0.
// No parameters. The thunk only steps the FFrame past the end-of-params marker, which C++ does not need.
if (!ServerSuicide_Validate())
{
	RPC_ValidateFailed(TEXT("ServerSuicide_Validate"));
	return;
}
ServerSuicide_Implementation();
