// Helper 0x1407c5360 (host UFortTabListWidgetBase). Asm: MOV EAX,[RCX+0x2c0]; SUB EAX,[RCX+0x2ec]; RET.
// Returns int32 [+0x2c0] minus int32 [+0x2ec] with 32-bit wrap. Caller GetVisibleTabCount (0x144dbe0f0) passes
// its own receiver in RCX and uses the result as a loop bound.
// lookup_offset.py: +0x2c0 lies inside reflected RegisteredTabsByID (Map, +0x2b8); +0x2ec has no covering member.
// The map's internal counter layout is not reflected, so both fields are read raw as int32. The meaning of the
// two counters is inferred from the caller only.
// CONFIDENCE: MEDIUM
const uint8* TabListBytes = reinterpret_cast<const uint8*>(TabListWidget);
const uint32 Value2C0 = static_cast<uint32>(*reinterpret_cast<const int32*>(TabListBytes + 0x2C0));
const uint32 Value2EC = static_cast<uint32>(*reinterpret_cast<const int32*>(TabListBytes + 0x2EC));
return static_cast<int32>(Value2C0 - Value2EC);
