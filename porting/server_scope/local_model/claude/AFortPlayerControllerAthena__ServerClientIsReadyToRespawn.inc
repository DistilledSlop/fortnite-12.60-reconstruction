// CONFIDENCE: HIGH
// Original address: 0x142a5ad50 (RPC thunk). No parameters.
if (!ServerClientIsReadyToRespawn_Validate())
{
	RPC_ValidateFailed(TEXT("ServerClientIsReadyToRespawn_Validate"));
	return;
}
ServerClientIsReadyToRespawn_Implementation();
