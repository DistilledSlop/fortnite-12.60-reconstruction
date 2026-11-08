// Helper 0x1407ef240 (AFortAthenaCreativePortal). Asm evidence (76 insns, all accounted for):
//   Num at [RCX+8] (0 -> return 0). Compares each 8-byte element against Key with
//   NativeWeakObjectPtrEquivalent1407327b0 (0x1407327b0) and compacts the kept runs to the front with
//   memmove (0x1458092f3). Stores the new Num (kept count) at [RCX+8] and returns the removed count.
// CONFIDENCE: MEDIUM
const int32 Count = Items.Num();
if (Count == 0)
{
    return 0;
}
uint8** Data = Items.GetData();
int32 KeptCount = 0;
int32 RunStart = 0;
bool bKeepRun = !UCreativeQuestComponent::NativeWeakObjectPtrEquivalent1407327b0(reinterpret_cast<const uint8*>(&Data[0]), Key);
for (int32 Index = 1; Index <= Count; ++Index)
{
    if (Index == Count)
    {
        if (bKeepRun)
        {
            const int32 RunLength = Index - RunStart;
            if (KeptCount != RunStart)
            {
                FMemory::Memmove(Data + KeptCount, Data + RunStart, static_cast<SIZE_T>(RunLength) * sizeof(uint8*));
            }
            KeptCount += RunLength;
        }
        break;
    }
    const bool bKeepIndex = !UCreativeQuestComponent::NativeWeakObjectPtrEquivalent1407327b0(reinterpret_cast<const uint8*>(&Data[Index]), Key);
    if (bKeepIndex != bKeepRun)
    {
        if (bKeepRun)
        {
            const int32 RunLength = Index - RunStart;
            if (KeptCount != RunStart)
            {
                FMemory::Memmove(Data + KeptCount, Data + RunStart, static_cast<SIZE_T>(RunLength) * sizeof(uint8*));
            }
            KeptCount += RunLength;
        }
        RunStart = Index;
        bKeepRun = bKeepIndex;
    }
}
// Store the new Num without touching the allocation (asm writes only [RCX+8]).
*reinterpret_cast<int32*>(reinterpret_cast<uint8*>(&Items) + 8) = KeptCount;
return Count - KeptCount;
