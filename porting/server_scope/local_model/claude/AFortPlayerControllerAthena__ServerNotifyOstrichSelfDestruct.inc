// CONFIDENCE: HIGH
// Original address: 0x142a5b840 (RPC thunk). No parameters.
if (!ServerNotifyOstrichSelfDestruct_Validate())
{
	RPC_ValidateFailed(TEXT("ServerNotifyOstrichSelfDestruct_Validate"));
	return;
}
ServerNotifyOstrichSelfDestruct_Implementation();
