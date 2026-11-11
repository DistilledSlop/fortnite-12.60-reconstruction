// CONFIDENCE: HIGH. Server RPC entry for swapping an item between two inventory owners. Validation runs first; a failed validation reports RPC_ValidateFailed and drops the call, otherwise the call is forwarded to the implementation.
// Original addresses: exec thunk 0x1428a62d0; validate = virtual slot 0x440 (0x1406801d0); implementation = virtual slot 0x448 (0x141d15ae0).
if (!ServerSwapItem_Validate(SourceOwner, TargetOwner, ItemGuid, Count))
{
	RPC_ValidateFailed(TEXT("ServerSwapItem_Validate"));
	return;
}
ServerSwapItem_Implementation(SourceOwner, TargetOwner, ItemGuid, Count);
