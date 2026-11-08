// Helper 0x144d7bab0 (UFortBaseButton host). Returns the registry child Inner = Table->[0x130] when both
// the table and Inner->[0x10] register UFortButtonStyle::StaticClass(); otherwise nullptr.
// Table = Button->[0x298]; a class registration check is: index = (int32)Class->[0x38], key = Class+0x30,
// valid when index <= Container->[0x38] (int32) and Container->[0x30][index] == key.
// asm check B repeats check A on the same unchanged table, so its failure edge (which would load absolute
// address 0x130 with RDX=0) cannot be reached after check A passed; it is collapsed here.
// CONFIDENCE: MEDIUM
if (Button == nullptr)
{
    return nullptr;
}
const uint8* LkTable = *reinterpret_cast<const uint8* const*>(reinterpret_cast<const uint8*>(Button) + 0x298);
if (LkTable == nullptr)
{
    return nullptr;
}
const uint8* LkClass = reinterpret_cast<const uint8*>(UFortButtonStyle::StaticClass());
if (LkClass == nullptr)
{
    return nullptr;
}
const uint8* LkKey = LkClass + 0x30;
int32 LkIdx = *reinterpret_cast<const int32*>(LkClass + 0x38);
if (LkIdx > *reinterpret_cast<const int32*>(LkTable + 0x38))
{
    return nullptr;
}
const uint8* const* LkSlots = *reinterpret_cast<const uint8* const* const*>(LkTable + 0x30);
if (LkSlots[static_cast<int64>(LkIdx)] != LkKey)
{
    return nullptr;
}
uint8* LkInner = *reinterpret_cast<uint8* const*>(LkTable + 0x130);
if (LkInner == nullptr)
{
    return nullptr;
}
const uint8* LkClass2 = reinterpret_cast<const uint8*>(UFortButtonStyle::StaticClass());
const uint8* LkKey2 = LkClass2 + 0x30;
const uint8* LkX = *reinterpret_cast<const uint8* const*>(LkInner + 0x10);
int32 LkIdx2 = *reinterpret_cast<const int32*>(LkClass2 + 0x38);
if (LkIdx2 > *reinterpret_cast<const int32*>(LkX + 0x38))
{
    return nullptr;
}
const uint8* const* LkXSlots = *reinterpret_cast<const uint8* const* const*>(LkX + 0x30);
if (LkXSlots[static_cast<int64>(LkIdx2)] != LkKey2)
{
    return nullptr;
}
return LkInner;
