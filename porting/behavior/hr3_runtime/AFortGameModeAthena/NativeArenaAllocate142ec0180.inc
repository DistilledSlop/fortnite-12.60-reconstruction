// Helper 0x142ec0180: arena bump allocation with FMemory::Malloc fallback. RCX=ArenaState (int32 Capacity @+0x0, uint8* Base @+0x8,
// uint8* Cursor @+0x10, uint8* HighWater @+0x18), EDX=RequestedSize, R8 is not read by the callee, R9B=bUseArena.
// Evidence: asm 0x142ec0180. Fit test: align16(Cursor)+SignExt(Size) <= Capacity+Base. Fit -> bump Cursor, raise HighWater, return aligned old Cursor.
// No fit with bUseArena -> HighWater bumped and Malloc. bUseArena == 0 -> FMemory::Malloc(align16(Size), 0) tail call (0x142bfb0f0).
// Callers: 0x142d68560 and 0x142d696a0 (R9B=1, EDX=0x248, R8 set but unread, result stored to a global and used as a pointer).
// CONFIDENCE: MEDIUM
(void)ReservedR8;
const uint32 AlignedSize = (static_cast<uint32>(RequestedSize) + 0xfu) & 0xfffffff0u;
if (!bUseArena)
{
    return static_cast<uint8*>(FMemory::Malloc(static_cast<SIZE_T>(AlignedSize), 0));
}
const int32 Capacity = *reinterpret_cast<const int32*>(ArenaState + 0x0);
uint8* const ArenaBase = *reinterpret_cast<uint8* const*>(ArenaState + 0x8);
uint8** const CursorSlot = reinterpret_cast<uint8**>(ArenaState + 0x10);
uint8** const HighWaterSlot = reinterpret_cast<uint8**>(ArenaState + 0x18);
const uintptr_t AlignedCursor = (reinterpret_cast<uintptr_t>(*CursorSlot) + 0xfu) & ~static_cast<uintptr_t>(0xfu);
const uintptr_t ProjectedEnd = AlignedCursor + static_cast<uintptr_t>(static_cast<intptr_t>(RequestedSize));
const uintptr_t CapacityLimit = static_cast<uintptr_t>(static_cast<intptr_t>(Capacity)) + reinterpret_cast<uintptr_t>(ArenaBase);
if (ProjectedEnd > CapacityLimit)
{
    const uintptr_t BumpedHighWater = (reinterpret_cast<uintptr_t>(*HighWaterSlot) + 0xfu) & ~static_cast<uintptr_t>(0xfu);
    *HighWaterSlot = reinterpret_cast<uint8*>(BumpedHighWater + static_cast<uintptr_t>(static_cast<intptr_t>(RequestedSize)));
    return static_cast<uint8*>(FMemory::Malloc(static_cast<SIZE_T>(AlignedSize), 0));
}
*CursorSlot = reinterpret_cast<uint8*>(ProjectedEnd);
if (reinterpret_cast<uintptr_t>(*HighWaterSlot) < ProjectedEnd)
{
    *HighWaterSlot = reinterpret_cast<uint8*>(ProjectedEnd);
}
return reinterpret_cast<uint8*>(AlignedCursor);
