// CONFIDENCE: HIGH for dispatch, MEDIUM for parameter order (reverse-of-reflection read order).
// Original exec thunk 0x142a4c540. Server RPC receiver: validate, then implementation.
// Validate slot 0x1040 resolves to Stub_ReturnTrue (0x1406801d0), so the validate always passes.
if (!ServerRemoveInventoryItem_Validate(ItemGuid, Count, bForceRemoveFromQuickBars, bForceRemoval, bForcePersistWhenEmpty))
{
    RPC_ValidateFailed(TEXT("ServerRemoveInventoryItem_Validate"));
    return;
}
ServerRemoveInventoryItem_Implementation(ItemGuid, Count, bForceRemoveFromQuickBars, bForceRemoval, bForcePersistWhenEmpty);   // slot 0x1048 -> 0x1423abed0
