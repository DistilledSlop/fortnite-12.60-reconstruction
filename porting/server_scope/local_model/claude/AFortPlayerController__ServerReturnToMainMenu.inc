// CONFIDENCE: HIGH for dispatch, MEDIUM for parameter order (reverse-of-reflection read order).
// Original exec thunk 0x142a4cf60. Server RPC receiver: validate, then implementation.
// Validate slot 0x1320 resolves to Stub_ReturnTrue (0x1406801d0), so the validate always passes.
if (!ServerReturnToMainMenu_Validate())
{
    RPC_ValidateFailed(TEXT("ServerReturnToMainMenu_Validate"));
    return;
}
ServerReturnToMainMenu_Implementation();   // slot 0x1328 -> 0x1423ac090
