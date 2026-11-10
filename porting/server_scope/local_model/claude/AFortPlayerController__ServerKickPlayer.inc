// CONFIDENCE: HIGH for dispatch, MEDIUM for parameter order (reverse-of-reflection read order).
// Original exec thunk 0x142a4bb90. Server RPC receiver: validate, then implementation.
// Validate slot 0xf20 resolves to Stub_ReturnTrue (0x1406801d0), so the validate always passes.
if (!ServerKickPlayer_Validate(Reason, PlayerToKick))
{
    RPC_ValidateFailed(TEXT("ServerKickPlayer_Validate"));
    return;
}
ServerKickPlayer_Implementation(Reason, PlayerToKick);   // slot 0xf28 -> 0x1423aba80
