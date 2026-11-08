// Helper 0x140690ee0: TArray capacity update for a 24-byte element. RCX=ArrayStorage (Data @+0x0, Max @+0xc), EDX=NewNum.
// Evidence: asm 0x140690ee0. NewNum*24 -> FMemory::QuantizeSize (0x142bfce10); quotient by 24 (magic multiply + shift 4) truncated to int32;
// NewMax = NewNum > quotient ? INT32_MAX : quotient; Max stored, then Data = FMemory::Realloc (0x142bfd200) when Data != null or NewMax != 0.
// Callers: 0x1422b5db0 (RCX=this+0x20c8, EDX=count), 0x144f65aa0 (RCX=local array, EDX=0).
// CONFIDENCE: HIGH
int32 NewMax = 0;
if (NewNum != 0)
{
    const SIZE_T QuantizedBytes = FMemory::QuantizeSize(static_cast<SIZE_T>(static_cast<int64>(NewNum)) * 24, 0);
    const int32 QuantizedCount = static_cast<int32>(static_cast<uint32>(QuantizedBytes / 24));
    NewMax = (NewNum > QuantizedCount) ? 0x7fffffff : QuantizedCount;
}
int32* MaxSlot = reinterpret_cast<int32*>(ArrayStorage + 0xc);
if (NewMax != *MaxSlot)
{
    *MaxSlot = NewMax;
    uint8** DataSlot = reinterpret_cast<uint8**>(ArrayStorage);
    if (*DataSlot != nullptr || NewMax != 0)
    {
        *DataSlot = static_cast<uint8*>(FMemory::Realloc(*DataSlot, static_cast<SIZE_T>(static_cast<int64>(NewMax)) * 24, 0));
    }
}
