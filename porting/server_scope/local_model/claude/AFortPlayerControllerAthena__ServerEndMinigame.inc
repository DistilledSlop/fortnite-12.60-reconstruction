// CONFIDENCE: HIGH
// Original address: 0x142a5b240 (RPC thunk). Single parameter bAbandon (bool, read as int32 != 0).
if (!ServerEndMinigame_Validate(bAbandon))
{
	RPC_ValidateFailed(TEXT("ServerEndMinigame_Validate"));
}
else
{
	ServerEndMinigame_Implementation(bAbandon);
}
