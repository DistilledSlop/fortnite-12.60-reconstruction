// CONFIDENCE: HIGH for dispatch, MEDIUM for parameter order (reverse-of-reflection read order).
// Original exec thunk 0x142a4c480. Server RPC receiver: validate, then implementation.
// Validate slot 0x1138 resolves to Stub_ReturnTrue (0x1406801d0), so the validate always passes.
if (!ServerRemoveDefender_Validate(DefenderTrap))
{
    RPC_ValidateFailed(TEXT("ServerRemoveDefender_Validate"));
    return;
}
ServerRemoveDefender_Implementation(DefenderTrap);   // slot 0x1140 -> 0x1423abe70
