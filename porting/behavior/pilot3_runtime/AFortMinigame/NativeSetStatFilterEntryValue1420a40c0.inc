// Helper 0x1420a40c0: find-or-add a 16-byte {UClass* key; int32 value} entry in an entry array and store the value.
// Evidence: RDX = TArray (Data@0, Num@+8, Max@+0xc; stride 0x10 = SHL 4); RCX is never read (callers pass the owning struct or
//   the array itself); R8 = key, passed by address ([RSP+0x40] is its spilled home) to the ported lookup 0x14207a350
//   (NativeFindStatFilterEntry14207a350); on a miss the array is grown by one (Num+1, grow when > Max = AddUninitialized(1))
//   and [entry+0] = R8; R9D (int32) is stored at [entry+8] in both cases; no return value.
// CONFIDENCE: MEDIUM
uint8* FoundEntry = AFortMinigame::NativeFindStatFilterEntry14207a350(EntryArray, &FilterClass);
if (FoundEntry == nullptr)
{
    TArray<TPair<UClass*, int64>>& EntryList = *reinterpret_cast<TArray<TPair<UClass*, int64>>*>(EntryArray);
    const int32 NewEntryIndex = EntryList.AddUninitialized(1);
    FoundEntry = reinterpret_cast<uint8*>(&EntryList[NewEntryIndex]);
    *reinterpret_cast<UClass**>(FoundEntry) = FilterClass;
}
*reinterpret_cast<int32*>(FoundEntry + 0x8) = NewValue;
