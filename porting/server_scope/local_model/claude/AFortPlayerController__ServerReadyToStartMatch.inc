// CONFIDENCE: HIGH for dispatch, MEDIUM for parameter order (reverse-of-reflection read order).
// Original exec thunk 0x142a4c360. Server RPC receiver: validate, then implementation.
// Validate slot 0x1340 resolves to Stub_ReturnTrue (0x1406801d0), so the validate always passes.
if (!ServerReadyToStartMatch_Validate())
{
    RPC_ValidateFailed(TEXT("ServerReadyToStartMatch_Validate"));
    return;
}
ServerReadyToStartMatch_Implementation();   // slot 0x1348 -> 0x1423abda0
