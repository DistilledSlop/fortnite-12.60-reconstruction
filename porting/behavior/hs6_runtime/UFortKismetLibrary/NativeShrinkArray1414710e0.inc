// Native 0x1414710e0: TArray slack/shrink policy for a 0x48-byte element array (Data @0x0, Num @0x8, Max @0xC).
// Asm evidence: slack bytes = (Max-Num)*0x48 compared with 0x4000; Num*3 >= Max*2 keeps Max; slack > 0x40 requantizes
// Num*0x48 through FMemory::QuantizeSize(size, 0) and divides by 0x48 (magic multiply verified against /72);
// when the count changes Max is stored and Data is reallocated via FMemory::Realloc(Data, count*0x48, 0)
// unless both Data and the new count are zero.
// CONFIDENCE: MEDIUM
const int64 ElementStride = 0x48;
const int32 ArrayNum = *reinterpret_cast<const int32*>(Array + 0x8);
const int32 ArrayMax = *reinterpret_cast<const int32*>(Array + 0xC);
const int32 ArraySlack = ArrayMax - ArrayNum;
const uint64 SlackBytes = static_cast<uint64>(static_cast<int64>(ArraySlack) * ElementStride);
int32 NewMax = ArrayNum;
if (SlackBytes < 0x4000 && ArrayNum * 3 >= ArrayMax * 2)
{
    NewMax = ArrayMax;
}
else if (ArraySlack > 0x40)
{
    if (ArrayNum > 0)
    {
        const uint64 QuantizedBytes = FMemory::QuantizeSize(static_cast<uint64>(static_cast<int64>(ArrayNum) * ElementStride), 0);
        NewMax = static_cast<int32>(QuantizedBytes / static_cast<uint64>(ElementStride));
    }
}
else if (ArrayNum != 0)
{
    NewMax = ArrayMax;
}
if (NewMax != ArrayMax)
{
    *reinterpret_cast<int32*>(Array + 0xC) = NewMax;
    void* ArrayData = *reinterpret_cast<void**>(Array);
    if (ArrayData != nullptr || NewMax != 0)
    {
        *reinterpret_cast<void**>(Array) = FMemory::Realloc(ArrayData, static_cast<SIZE_T>(static_cast<int64>(NewMax) * ElementStride), 0);
    }
}
