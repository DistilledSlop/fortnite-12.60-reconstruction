// Behaviour snippet: AFortBroadcastRemoteClientInfo::ServerSetPlayerEditTileData_Implementation (body 0x1409a9160, thunk 0x14284fca0).
// Offset evidence: RemoteEditTileData TArray<int32> at object +0x278 (Data), Num at +0x280, Max at +0x284 (this_delta_total 632).
// Asm: self-assignment guard (CMP RCX,RDX); store source Num to +0x280; resize via 0x14072b4b0 (NativeResizeArrayCapacity14072b4b0) when Num or old Max is nonzero, then memcpy Num*4 bytes; otherwise Max = Num.
// CONFIDENCE: HIGH
if (&RemoteEditTileData == &EditTileData)
{
    return;
}
uint8* Raw = reinterpret_cast<uint8*>(this);
const int32 NewNum = EditTileData.Num();
const int32 PreviousMax = *reinterpret_cast<const int32*>(Raw + 0x284);
*reinterpret_cast<int32*>(Raw + 0x280) = NewNum;
if (NewNum != 0 || PreviousMax != 0)
{
    AFortBroadcastRemoteClientInfo::NativeResizeArrayCapacity14072b4b0(RemoteEditTileData, NewNum, PreviousMax);
    FMemory::Memcpy(RemoteEditTileData.GetData(), EditTileData.GetData(), static_cast<SIZE_T>(NewNum) * sizeof(int32));
}
else
{
    *reinterpret_cast<int32*>(Raw + 0x284) = NewNum;
}
