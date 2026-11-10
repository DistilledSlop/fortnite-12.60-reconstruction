// CONFIDENCE: HIGH
// Original address: 0x142a5b020 (RPC thunk). Single parameter DBNOPawn (object).
if (!ServerDBNOReviveStarted_Validate(DBNOPawn))
{
	RPC_ValidateFailed(TEXT("ServerDBNOReviveStarted_Validate"));
	return;
}
ServerDBNOReviveStarted_Implementation(DBNOPawn);
