// Helper 0x1450e86c0, host UFortSquadSlotSelectorButton (static).
// Evidence: MOV RAX,[RCX+0x8]; MOV [RDX],RAX; MOV EAX,[RCX+0x10]; MOV [R8],EAX; RET.
// Context is an interior struct pointer (callers pass obj+0xb70). Writes a 64-bit value from +0x8 and an int32 from +0x10.
// CONFIDENCE: MEDIUM
*OutKey = *reinterpret_cast<const uint64*>(Context + 0x8);
*OutIndex = *reinterpret_cast<const int32*>(Context + 0x10);
