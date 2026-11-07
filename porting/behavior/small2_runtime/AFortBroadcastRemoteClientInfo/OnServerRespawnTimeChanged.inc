// Behaviour snippet: AFortBroadcastRemoteClientInfo::OnServerRespawnTimeChanged (body only; spliced into the generated member function).
// Original address: 0x1424b5c40
// Evidence: Role (0xf0) compared to 3.
// Evidence: Gate byte [RCX+0x268] is NativeRecoveredByte_0x268 (gap).
// Evidence: Store dword [RCX+0x2e0] (XMM1) is reflected member RemoteRespawnTime (Float).
// CONFIDENCE: HIGH
if (static_cast<uint8>(Role) == 3 && NativeRecoveredByte_0x268 != 0)
{
    RemoteRespawnTime = InRespawnTime;
}
