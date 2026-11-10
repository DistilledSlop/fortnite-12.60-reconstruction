// CONFIDENCE: HIGH for dispatch, MEDIUM for parameter order (reverse-of-reflection read order).
// Original exec thunk 0x142a4bf40. Server RPC receiver: validate, then implementation.
// Validate slot 0xdb0 resolves to Stub_ReturnTrue (0x1406801d0), so the validate always passes.
if (!ServerNotifyMeshNetPlayer_Validate(Tag))
{
    RPC_ValidateFailed(TEXT("ServerNotifyMeshNetPlayer_Validate"));
    return;
}
ServerNotifyMeshNetPlayer_Implementation(Tag);   // slot 0xdb8 -> 0x1423abbe0
