// CONFIDENCE: HIGH
// Original address: 0x142a5ad00 (RPC thunk). No parameters.
if (!ServerClearSkydiveLeader_Validate())
{
	RPC_ValidateFailed(TEXT("ServerClearSkydiveLeader_Validate"));
	return;
}
ServerClearSkydiveLeader_Implementation();
