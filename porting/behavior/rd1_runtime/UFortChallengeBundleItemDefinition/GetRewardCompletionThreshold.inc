// Original body address: 0x141caaa70 (thunk 0x14285cbc0). Body asm: 27 instructions.
// Offset evidence: BundleCompletionRewards (reflected Array at +0x3c8, Num +0x3d0; element stride 0x18: int32 +0x0, inner data +0x8, inner Num +0x10; inner stride 0x80 with byte +0x79).
// Fallback value is the int32 at +0x390, which is QuestInfos.Num() (reflected Array at +0x388).
// CONFIDENCE: MEDIUM
const int32 OuterNum = BundleCompletionRewards.Num();
const uint8* Outer = *reinterpret_cast<const uint8* const*>(reinterpret_cast<const uint8*>(this) + 0x3c8);
for (int32 o = 0; o < OuterNum; ++o)
{
    const uint8* OuterElement = Outer + static_cast<int64>(o) * 0x18;
    const uint8* Inner = *reinterpret_cast<const uint8* const*>(OuterElement + 0x8);
    const int32 InnerNum = *reinterpret_cast<const int32*>(OuterElement + 0x10);
    for (int32 k = 0; k < InnerNum; ++k)
    {
        const uint8 Status = Inner[static_cast<int64>(k) * 0x80 + 0x79];
        if (static_cast<uint8>(Status - 1) > 1)
        {
            const int32 Value = *reinterpret_cast<const int32*>(OuterElement);
            return Value != -1 ? Value : QuestInfos.Num();
        }
    }
}
return QuestInfos.Num();
