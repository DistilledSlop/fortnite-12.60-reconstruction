// Helper 0x14069eac0 (UCreativeQuestComponent), asm porting/generated/native-cap-direct/0x14069eac0.asm.
// TArray<int32> capacity hysteresis: Data at +0, Num at +8, Max at +0xc (4-byte elements). Callers pass &array after removals.
// Slack = Max - Num. Resize when slack*4 >= 0x4000, or when 3*Num < 2*Max and slack > 0x40:
// new capacity = QuantizeSize(Num*4, 0) / 4 when Num > 0, otherwise Num. Otherwise capacity is Max, except Num == 0 gives 0.
// When capacity changes: store new Max, then Data = FMemory::Realloc(Data, NewMax*4, 0) if Data is non-null or NewMax is non-zero.
// Callee evidence: 0x142bfce10 = FMemory::QuantizeSize, 0x142bfd200 = FMemory::Realloc (callee_hints).
// CONFIDENCE: HIGH
const int32 Num = Array.Num();
int32* MaxSlot = reinterpret_cast<int32*>(reinterpret_cast<uint8*>(&Array) + 0xC);
void*& DataSlot = *reinterpret_cast<void**>(&Array);
const int32 Max = *MaxSlot;
const int32 Slack = Max - Num;
const bool bLargeSlack = static_cast<int64>(Slack) * 4 >= 0x4000;
const bool bNumBelowTwoThirds = Num * 3 < Max * 2;
int32 NewCapacity = Max;
if (bLargeSlack || (bNumBelowTwoThirds && Slack > 0x40))
{
    NewCapacity = Num > 0 ? static_cast<int32>(FMemory::QuantizeSize(static_cast<SIZE_T>(Num) * 4, 0) >> 2) : Num;
}
else if (Num == 0)
{
    NewCapacity = 0;
}
if (NewCapacity != Max)
{
    *MaxSlot = NewCapacity;
    if (DataSlot != nullptr || NewCapacity != 0)
    {
        DataSlot = FMemory::Realloc(DataSlot, static_cast<SIZE_T>(static_cast<int64>(NewCapacity) * 4), 0);
    }
}
