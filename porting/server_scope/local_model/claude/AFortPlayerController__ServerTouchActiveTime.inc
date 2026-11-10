// CONFIDENCE: HIGH
// Original: exec RPC wrapper 0x142a4dee0. Validate slot -> 0x1406801d0. Implementation slot -> 0x1423ad120.
// No parameters.
if (!ServerTouchActiveTime_Validate())
{
	RPC_ValidateFailed(TEXT("ServerTouchActiveTime_Validate"));
	return;
}
ServerTouchActiveTime_Implementation();
