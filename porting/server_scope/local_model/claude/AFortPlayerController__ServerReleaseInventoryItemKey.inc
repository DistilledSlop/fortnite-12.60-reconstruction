// CONFIDENCE: HIGH for dispatch, MEDIUM for parameter order (reverse-of-reflection read order).
// Original exec thunk 0x142a4c3b0. Server RPC receiver: validate, then implementation.
// Validate slot 0x1050 resolves to Stub_ReturnTrue (0x1406801d0), so the validate always passes.
if (!ServerReleaseInventoryItemKey_Validate(ItemGuid))
{
    RPC_ValidateFailed(TEXT("ServerReleaseInventoryItemKey_Validate"));
    return;
}
ServerReleaseInventoryItemKey_Implementation(ItemGuid);   // slot 0x1058 -> 0x1423abdb0
