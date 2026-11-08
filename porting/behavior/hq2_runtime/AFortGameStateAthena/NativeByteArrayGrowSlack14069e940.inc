// Helper 0x14069e940: TArray slack growth, element size 1 bytes (asm: SHL/LEA scale none).
// Array layout: Data pointer at +0, Num int32 at +0x8, Max int32 at +0xc.
// Evidence: if Max == 0 cap = max(Num,1) else cap = Num + (3*Num)>>3; Bytes = FMemory::QuantizeSize(cap*1, 0);
// NewMax = Bytes/1 (INT32_MAX if Num > NewMax); Max stored; Data = Realloc(Data, NewMax*1, 0) when Data != null or NewMax != 0.
// Callers (UUACNetworkComponent / ABuildingFoundation) set Num before the call and pass RCX=&array (EDX ignored by this helper).

uint8* ArrBase = static_cast<uint8*>(Array);
void** ArrData = reinterpret_cast<void**>(ArrBase);
const int32 ArrNum = *reinterpret_cast<const int32*>(ArrBase + 0x8);
const int32 ArrMax = *reinterpret_cast<const int32*>(ArrBase + 0xc);
const uint64 NumU = static_cast<uint64>(static_cast<int64>(ArrNum));
uint64 Cap = (ArrMax == 0) ? (NumU > 1 ? NumU : 1) : (NumU + ((NumU * 3) >> 3));
const uint64 Bytes = FMemory::QuantizeSize(Cap, 0);
int32 NewMax = static_cast<int32>(Bytes);
if (ArrNum > NewMax)
{
    NewMax = 0x7fffffff;
}
*reinterpret_cast<int32*>(ArrBase + 0xc) = NewMax;
if (*ArrData != nullptr || NewMax != 0)
{
    *ArrData = FMemory::Realloc(*ArrData, static_cast<SIZE_T>(static_cast<int64>(NewMax)) * 1, 0);
}
// CONFIDENCE: HIGH
