// Helper 0x140694750: TArray capacity set, element stride 12 bytes (LEA*3, SHL 2).
// Raw layout: Data at +0, Max int32 at +0xc. Num is not written by this helper.
// Calls: FMemory::QuantizeSize 0x142bfce10 (RCX=bytes, EDX=0); FMemory::Realloc 0x142bfd200 (R8D=0).
// Quantized count = QuantizeSize(bytes)/12 (magic 0xAAAAAAAAAAAAAAAB, >>3); NewCount==0 gives Max 0.
// CONFIDENCE: HIGH
uint8* ArrayBytes = reinterpret_cast<uint8*>(Array);
int32 NewMax = 0;
if (NewCount != 0)
{
    const uint64 QuantizedBytes = FMemory::QuantizeSize(static_cast<uint64>(static_cast<int64>(NewCount)) * 12, 0);
    const uint64 CapacityFromBytes = QuantizedBytes / 12;
    NewMax = (NewCount > static_cast<int32>(CapacityFromBytes))
        ? 2147483647
        : static_cast<int32>(CapacityFromBytes);
}
int32* MaxSlot = reinterpret_cast<int32*>(ArrayBytes + 0xc);
if (NewMax != *MaxSlot)
{
    *MaxSlot = NewMax;
    void** DataPtr = reinterpret_cast<void**>(ArrayBytes);
    void* ExistingData = *DataPtr;
    if (ExistingData != nullptr || NewMax != 0)
    {
        *DataPtr = FMemory::Realloc(ExistingData, static_cast<uint64>(static_cast<int64>(NewMax)) * 12, 0);
    }
}
