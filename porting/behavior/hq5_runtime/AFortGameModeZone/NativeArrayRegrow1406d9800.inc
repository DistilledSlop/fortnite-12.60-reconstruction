// Helper 0x1406d9800: TArray capacity regrow, element stride 20 bytes (LEA*5, SHL 2).
// Raw layout: Data at +0, Num int32 at +8, Max int32 at +0xc.
// Calls: FMemory::QuantizeSize 0x142bfce10 (RCX=bytes, EDX=0); FMemory::Realloc 0x142bfd200 (R8D=0).
// Quantized count = QuantizeSize(bytes)/20 (magic 0xCCCCCCCCCCCCCCCD, >>4); new Max is INT32_MAX when Num exceeds it.
// CONFIDENCE: HIGH
uint8* ArrayBytes = reinterpret_cast<uint8*>(Array);
void** DataPtr = reinterpret_cast<void**>(ArrayBytes);
const int32 CurrentNum = *reinterpret_cast<const int32*>(ArrayBytes + 0x8);
const int32 CurrentMax = *reinterpret_cast<const int32*>(ArrayBytes + 0xc);
const uint64 NumWide = static_cast<uint64>(static_cast<int64>(CurrentNum));
uint64 CandidateCount;
if (CurrentMax == 0)
{
    CandidateCount = (NumWide > 1) ? NumWide : 1;
}
else
{
    CandidateCount = NumWide + ((NumWide * 3) >> 3);
}
const uint64 ByteCount = CandidateCount * 20;
const uint64 QuantizedBytes = FMemory::QuantizeSize(ByteCount, 0);
const uint64 CapacityFromBytes = QuantizedBytes / 20;
const int32 NewMax = (CurrentNum > static_cast<int32>(CapacityFromBytes))
    ? 2147483647
    : static_cast<int32>(CapacityFromBytes);
*reinterpret_cast<int32*>(ArrayBytes + 0xc) = NewMax;
void* ExistingData = *DataPtr;
if (ExistingData != nullptr || NewMax != 0)
{
    *DataPtr = FMemory::Realloc(ExistingData, static_cast<uint64>(static_cast<int64>(NewMax)) * 20, 0);
}
