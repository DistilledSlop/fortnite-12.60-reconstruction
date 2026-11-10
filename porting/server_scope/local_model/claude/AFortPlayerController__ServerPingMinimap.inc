// CONFIDENCE: HIGH for dispatch, MEDIUM for parameter order (reverse-of-reflection read order).
// Original exec thunk 0x142a4c0f0. Server RPC receiver: validate, then implementation.
// Validate slot 0x12a8 resolves to Stub_ReturnTrue (0x1406801d0), so the validate always passes.
if (!ServerPingMinimap_Validate(WorldLocation, PingColor, RequiredTeam))
{
    RPC_ValidateFailed(TEXT("ServerPingMinimap_Validate"));
    return;
}
ServerPingMinimap_Implementation(WorldLocation, PingColor, RequiredTeam);   // slot 0x12b0 -> 0x1423abd50
