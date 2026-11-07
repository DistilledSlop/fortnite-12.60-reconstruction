// Original body address: 0x141611890 (thunk 0x14278b340). Body asm: 31 instructions.
// Offset evidence: class has no reflected members, so this+0x238 (TArray data, 8-byte elements) and this+0x240 (int32 Num) are read raw. Ability+0x18 is the 8-byte name key (UObject name storage, raw).
// Result is true when any element equals the key (both 32-bit halves), else false. Null Ability gives key 0.
// CONFIDENCE: HIGH
const uint8* Raw = reinterpret_cast<const uint8*>(this);
uint64 Key = 0;
if (Ability != nullptr)
{
    Key = *reinterpret_cast<const uint64*>(reinterpret_cast<const uint8*>(Ability) + 0x18);
}
const int32 Count = *reinterpret_cast<const int32*>(Raw + 0x240);
const uint64* Entries = *reinterpret_cast<const uint64* const*>(Raw + 0x238);
for (int32 i = 0; i < Count; ++i)
{
    if (Entries[i] == Key) { return true; }
}
return false;
