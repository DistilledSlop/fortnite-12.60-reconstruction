// Behaviour snippet: AFortBroadcastRemoteClientInfo::OnServerPoiTagIDChanged (body only; spliced into the generated member function).
// Original address: 0x1424b5c00
// Evidence: Role (0xf0) compared to 3.
// Evidence: Gate byte [RCX+0x268] is NativeRecoveredByte_0x268 (gap).
// Evidence: Store word [RCX+0x2e4] is reflected member RemotePoiTagID (Int16).
// CONFIDENCE: HIGH
if (static_cast<uint8>(GetLocalRole()) == 3 && NativeRecoveredByte_0x268 != 0)
{
    RemotePoiTagID = InPoiTagID;
}
