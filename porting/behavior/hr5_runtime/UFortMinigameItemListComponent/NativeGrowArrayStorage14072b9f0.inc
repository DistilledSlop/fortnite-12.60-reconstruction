// Helper 0x14072b9f0 (host UFortMinigameItemListComponent): grows a TArray of 96-byte elements. Callers: MakeSpawnParamsList 0x14209b220, Finished_CutDuplicateStartInteractingOnServer 0x142668810.
// Array layout (RCX): Data @+0 (pointer), Num @+8 (int32), Max @+12 (int32). EDX is unused by the helper.
// Asm: Max==0 -> requested=max(Num,1); else requested=Num+(3*Num>>3). Bytes=FMemory::QuantizeSize(requested*96,0).
// Max=bytes/96; if Num > Max, Max=INT32_MAX (signed CMOVG clamp). Realloc Data to Max*96 bytes unless Data==null and Max==0.
// CONFIDENCE: HIGH
const int32 Count = *reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(ArrayPtr) + 8);
int32* MaxSlot = reinterpret_cast<int32*>(reinterpret_cast<uint8*>(ArrayPtr) + 12);
uint8** DataSlot = reinterpret_cast<uint8**>(ArrayPtr);
int64 RequestedCount;
if (*MaxSlot == 0)
{
	RequestedCount = (Count > 1) ? static_cast<int64>(Count) : 1;
}
else
{
	RequestedCount = static_cast<int64>(Count) + ((static_cast<int64>(Count) * 3) >> 3);
}
const SIZE_T QuantizedBytes = FMemory::QuantizeSize(static_cast<SIZE_T>(RequestedCount) * 96, 0);
const int64 CapacityCount = static_cast<int64>(QuantizedBytes / 96);
int32 NewMax = static_cast<int32>(CapacityCount);
if (Count > NewMax)
{
	NewMax = 0x7fffffff;
}
*MaxSlot = NewMax;
if (*DataSlot != nullptr || NewMax != 0)
{
	*DataSlot = static_cast<uint8*>(FMemory::Realloc(*DataSlot, static_cast<SIZE_T>(static_cast<int64>(NewMax)) * 96, 0));
}
