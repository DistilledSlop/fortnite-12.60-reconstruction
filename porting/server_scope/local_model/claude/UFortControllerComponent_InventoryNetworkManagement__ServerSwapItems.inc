// CONFIDENCE: MEDIUM RPC dispatch shape is certain: validate, then implementation. Argument order follows the FFrame step order (SourceOwner, TargetOwner, then the ItemGuids array); the validate and implementation calls share that order.
// Original: exec 0x1428a6460 (decompile captures/67db0021db174d4891ebfa82569ce31b/0x1428a6460.c)
// Validate slot 0x430 resolves to the shared Stub_ReturnTrue (0x1406801d0), so the RPC_ValidateFailed branch is not reachable in the base class. Implementation slot 0x438 resolves to 0x141d15b80, which forwards to FUN_14201e9e0 (the swap itself).
if (!ServerSwapItems_Validate(SourceOwner, TargetOwner, ItemGuids))
{
	RPC_ValidateFailed(TEXT("ServerSwapItems_Validate"));
	return;
}
ServerSwapItems_Implementation(SourceOwner, TargetOwner, ItemGuids);
