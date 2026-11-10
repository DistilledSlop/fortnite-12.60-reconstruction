// CONFIDENCE: HIGH
// Original address: 0x142a5af60 (RPC thunk). Single parameter DBNOPawn (object).
if (!ServerDBNOReviveInterrupted_Validate(DBNOPawn))
{
	RPC_ValidateFailed(TEXT("ServerDBNOReviveInterrupted_Validate"));
	return;
}
ServerDBNOReviveInterrupted_Implementation(DBNOPawn);
