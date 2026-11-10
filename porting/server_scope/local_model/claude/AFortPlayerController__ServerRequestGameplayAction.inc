// CONFIDENCE: HIGH for dispatch, MEDIUM for parameter order (reverse-of-reflection read order).
// Original exec thunk 0x142a4ceb0. Server RPC receiver: validate, then implementation.
// Validate slot 0x1490 resolves to Stub_ReturnTrue (0x1406801d0), so the validate always passes.
if (!ServerRequestGameplayAction_Validate(RequestedAction))
{
    RPC_ValidateFailed(TEXT("ServerRequestGameplayAction_Validate"));
    return;
}
ServerRequestGameplayAction_Implementation(RequestedAction);   // slot 0x1498 -> 0x1423ac080
