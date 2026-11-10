// CONFIDENCE: HIGH for dispatch, MEDIUM for parameter order (reverse-of-reflection read order).
// Original exec thunk 0x142a4c720. Server RPC receiver: validate, then implementation.
// Validate slot 0x1010 resolves to Stub_ReturnTrue (0x1406801d0), so the validate always passes.
if (!ServerRemoveInventoryStateValue_Validate(ItemGuid, StateValueType))
{
    RPC_ValidateFailed(TEXT("ServerRemoveInventoryStateValue_Validate"));
    return;
}
ServerRemoveInventoryStateValue_Implementation(ItemGuid, StateValueType);   // slot 0x1018 -> 0x1423a8370
