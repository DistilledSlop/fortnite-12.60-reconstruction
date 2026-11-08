// Helper 0x1407f1e50 (AFortAthenaMutator_Crucible). Windows x64: RCX = TArray storage (Data +0x0, Num +0x8, Max +0xc), RDX unused.
// Callers (0x1410abe60, 0x144afb0d0) store Num+1 first and call only when the new Num exceeds Max (ResizeGrow).
// Element stride 72 bytes (LEA x9 then SHL 3). Growth: Max==0 -> max(Num,1); else Num + (3*Num)>>3.
// NewMax = QuantizeSize(GrowCount*72, 0) / 72; if Num > NewMax then NewMax = INT32_MAX (CMOVG). Max is written before the realloc.
// Data is reallocated when Data != null or NewMax != 0: FMemory::Realloc(Data, NewMax*72, 0).
// CONFIDENCE: HIGH
uint8* ArrayStorageBytes = static_cast<uint8*>(ArrayStorage);
const int32 ArrayNum = *reinterpret_cast<const int32*>(ArrayStorageBytes + 0x8);
const int32 ArrayMax = *reinterpret_cast<const int32*>(ArrayStorageBytes + 0xc);
int64 GrowCount = 0;
if (ArrayMax == 0)
{
    GrowCount = (static_cast<int64>(ArrayNum) > 1) ? static_cast<int64>(ArrayNum) : 1;
}
else
{
    GrowCount = static_cast<int64>(ArrayNum) + ((static_cast<int64>(ArrayNum) * 3) >> 3);
}
const SIZE_T GrowBytes = static_cast<SIZE_T>(GrowCount) * 72;
const uint64 QuantizedBytes = FMemory::QuantizeSize(GrowBytes, 0);
int32 NewMax = static_cast<int32>(QuantizedBytes / 72);
if (ArrayNum > NewMax)
{
    NewMax = 0x7fffffff;
}
*reinterpret_cast<int32*>(ArrayStorageBytes + 0xc) = NewMax;
void* ArrayData = *reinterpret_cast<void**>(ArrayStorageBytes);
if (ArrayData != nullptr || NewMax != 0)
{
    ArrayData = FMemory::Realloc(ArrayData, static_cast<SIZE_T>(NewMax) * 72, 0);
    *reinterpret_cast<void**>(ArrayStorageBytes) = ArrayData;
}
