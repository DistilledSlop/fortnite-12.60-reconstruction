// Helper 0x140690fd0: TArray ResizeTo(NewMax), element size 40 bytes (asm: LEA*5 then SHL 3).
// Array layout: Data pointer at +0, Max int32 at +0xc.
// Evidence: if NewMax != 0: cap = QuantizeSize(NewMax*40,0)/40, INT32_MAX if NewMax > cap. If cap != Max: Max = cap;
// Data = Realloc(Data, cap*40, 0) when Data != null or cap != 0. Callers (SetPropertyOverrideValues, SendSpawnLootAnalytics) store Num then pass EDX=new Max.
uint8* ArrBase = static_cast<uint8*>(Array);
void** ArrData = reinterpret_cast<void**>(ArrBase);
int32 ReqMax = NewMax;
if (ReqMax != 0)
{
    const uint64 Bytes = FMemory::QuantizeSize(static_cast<uint64>(static_cast<int64>(ReqMax)) * 40, 0);
    ReqMax = static_cast<int32>(Bytes / 40);
    if (NewMax > ReqMax)
    {
        ReqMax = 0x7fffffff;
    }
}
if (ReqMax != *reinterpret_cast<const int32*>(ArrBase + 0xc))
{
    *reinterpret_cast<int32*>(ArrBase + 0xc) = ReqMax;
    if (*ArrData != nullptr || ReqMax != 0)
    {
        *ArrData = FMemory::Realloc(*ArrData, static_cast<SIZE_T>(static_cast<int64>(ReqMax)) * 40, 0);
    }
}
// CONFIDENCE: HIGH
