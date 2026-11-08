// Helper 0x142d66000: RCX=HandleState (uint8* Object @+0x0, byte bUseSecondary @+0x8), RDX=OutValue sret slot. Writes *OutValue and returns OutValue (RAX=RDX).
// Evidence: asm 0x142d66000. bUseSecondary != 0 -> Object->[+0x18] (no null check in asm); else Object != null -> Object->[+0x28]; else null.
// Caller: 0x142e5d3b0 (RCX=local struct, RDX=local slot, uses returned RAX as a pointer).
// CONFIDENCE: MEDIUM
uint8* const HandleObject = *reinterpret_cast<uint8* const*>(HandleState);
void* ResolvedValue = nullptr;
if (*(HandleState + 0x8) != 0)
{
    ResolvedValue = *reinterpret_cast<void* const*>(HandleObject + 0x18);
}
else if (HandleObject != nullptr)
{
    ResolvedValue = *reinterpret_cast<void* const*>(HandleObject + 0x28);
}
*OutValue = ResolvedValue;
return OutValue;
