// CONFIDENCE: HIGH
// Original address: 0x142a71150 (exec thunk). Thunk read order: VoteType (enum, 1 byte), then VoteDecision (int).
// Inventory also lists UnderlyingType (Byte); the thunk does not read it, so it is not a runtime argument.
// _Validate: vtable slot 0x20c0 -> 0x1406801d0 = STUB (returns true).
// _Implementation: vtable slot 0x20c8 -> 0x1423e9be0 (ported in AFortPlayerControllerZone__ServerSubmitGameplayVote_Implementation.inc).
if (!ServerSubmitGameplayVote_Validate(VoteType, VoteDecision))
{
	RPC_ValidateFailed(TEXT("ServerSubmitGameplayVote_Validate"));
	return;
}
ServerSubmitGameplayVote_Implementation(VoteType, VoteDecision);
