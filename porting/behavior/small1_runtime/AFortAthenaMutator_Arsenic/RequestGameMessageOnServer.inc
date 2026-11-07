// Original address: 0x141ab9620 (AFortAthenaMutator_Arsenic::RequestGameMessageOnServer)
// Offset evidence: [RCX+0xf0] is Role (reflected); [RCX+0x260] is CachedGameState (reflected Object); dword +0x1f00 on the pointee written raw.
// CONFIDENCE: MEDIUM
const uint8 MsgIdValue = static_cast<uint8>(MsgId);
if (static_cast<uint8>(Role) != 3 || CachedGameState == nullptr || MsgIdValue == 0)
{
    return;
}
*reinterpret_cast<int32*>(reinterpret_cast<uint8*>(CachedGameState) + 0x1f00) = static_cast<int32>(MsgIdValue);
