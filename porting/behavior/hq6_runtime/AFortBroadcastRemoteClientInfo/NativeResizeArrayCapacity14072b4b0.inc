// Helper 0x14072b4b0 (AFortBroadcastRemoteClientInfo). Asm evidence (all instructions accounted for):
//   capacity = NewNum; if NewNum != 0: capacity = QuantizeSize(NewNum*4, 0) / 4, clamped to INT32_MAX when NewNum exceeds it (0x142bfce10).
//   if capacity != PreviousMax and (Data != null or capacity != 0): Data = Realloc(Data, capacity*4, 0) (0x142bfd200).
//   Max at [RCX+0xc] = capacity.
// CONFIDENCE: HIGH
int32 Capacity = NewNum;
if (NewNum != 0)
{
    const SIZE_T QuantizedBytes = FMemory::QuantizeSize(static_cast<SIZE_T>(static_cast<int64>(NewNum)) << 2, 0);
    const int32 QuantizedCount = static_cast<int32>(QuantizedBytes >> 2);
    Capacity = (NewNum > QuantizedCount) ? 0x7fffffff : QuantizedCount;
}
if (Capacity != PreviousMax)
{
    void* CurrentData = reinterpret_cast<void*>(Array.GetData());
    if (CurrentData != nullptr || Capacity != 0)
    {
        void* NewData = FMemory::Realloc(CurrentData, static_cast<SIZE_T>(static_cast<int64>(Capacity)) << 2, 0);
        *reinterpret_cast<void**>(reinterpret_cast<uint8*>(&Array)) = NewData;
    }
}
*reinterpret_cast<int32*>(reinterpret_cast<uint8*>(&Array) + 12) = Capacity;
