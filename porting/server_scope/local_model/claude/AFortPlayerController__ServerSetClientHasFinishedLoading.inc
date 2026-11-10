// CONFIDENCE: HIGH for dispatch, MEDIUM for parameter order (reverse-of-reflection read order).
// Original exec thunk 0x142a4d220. Server RPC receiver: validate, then implementation.
// Validate slot 0x1360 resolves to Stub_ReturnTrue (0x1406801d0), so the validate always passes.
if (!ServerSetClientHasFinishedLoading_Validate(bInHasFinishedLoading))
{
    RPC_ValidateFailed(TEXT("ServerSetClientHasFinishedLoading_Validate"));
    return;
}
ServerSetClientHasFinishedLoading_Implementation(bInHasFinishedLoading);   // slot 0x1368 -> 0x1423ac490
