// Native helper 0x140690c30 (AFortAIGoalManager static). TArray slack recompute, element size 24 bytes.
// Layout from asm: Data at +0x0, Num int32 at +0x8, Max int32 at +0xc.
// Max==0 -> wanted = max(Num,1); Max!=0 -> wanted = Num + (3*Num)>>3 (LEA/SHR/ADD).
// Bytes = wanted*24 -> FMemory::QuantizeSize(Bytes,0); NewMax = q/24 (MUL 0xAAAAAAAAAAAAAAAB, SHR 4);
// if Num > NewMax (signed) NewMax = INT32_MAX. Store Max; if Data!=0 or NewMax!=0, Data = Realloc(Data, NewMax*24, 0).
// CONFIDENCE: MEDIUM
uint8* ArrayBytes = reinterpret_cast<uint8*>(ArrayStorage);
const int32 OldNum = *reinterpret_cast<const int32*>(ArrayBytes + 0x8);
const int32 OldMax = *reinterpret_cast<const int32*>(ArrayBytes + 0xc);
uint64 WantedElems = 1;
if (OldMax != 0)
{
	WantedElems = static_cast<uint64>(static_cast<int64>(OldNum) + ((static_cast<int64>(OldNum) * 3) >> 3));
}
else if (static_cast<uint64>(static_cast<int64>(OldNum)) > WantedElems)
{
	WantedElems = static_cast<uint64>(static_cast<int64>(OldNum));
}
const uint64 WantedBytes = WantedElems * 24;
const SIZE_T QuantizedBytes = FMemory::QuantizeSize(static_cast<SIZE_T>(WantedBytes), 0);
// Equivalent to the MUL/SHR 4 sequence for quantized sizes (multiples of 8).
int32 NewElemMax = static_cast<int32>(QuantizedBytes / 24);
if (OldNum > NewElemMax)
{
	NewElemMax = 0x7fffffff;
}
*reinterpret_cast<int32*>(ArrayBytes + 0xc) = NewElemMax;
void* DataPtr = *reinterpret_cast<void**>(ArrayBytes);
if (DataPtr != nullptr || NewElemMax != 0)
{
	*reinterpret_cast<void**>(ArrayBytes) = FMemory::Realloc(DataPtr, static_cast<SIZE_T>(static_cast<int64>(NewElemMax)) * 24, 0);
}
