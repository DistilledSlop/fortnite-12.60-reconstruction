// CONFIDENCE: HIGH for dispatch, MEDIUM for parameter order (reverse-of-reflection read order).
// Original exec thunk 0x142a4bd70. Server RPC receiver: validate, then implementation.
// Validate slot 0x1350 resolves to Stub_ReturnTrue (0x1406801d0), so the validate always passes.
if (!ServerLoadingScreenDropped_Validate())
{
    RPC_ValidateFailed(TEXT("ServerLoadingScreenDropped_Validate"));
    return;
}
ServerLoadingScreenDropped_Implementation();   // slot 0x1358 -> 0x1423abb20
