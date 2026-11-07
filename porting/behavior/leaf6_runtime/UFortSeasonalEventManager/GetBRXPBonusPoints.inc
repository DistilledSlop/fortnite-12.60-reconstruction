// Behaviour snippet: UFortSeasonalEventManager::GetBRXPBonusPoints (body only; spliced into the generated member function).
// Original address: 0x14224fe60 (asm: MOV RAX,[RCX+0xc0]; MOV RAX,[RAX+0x38]; RET). Thunk 0x142ad6d30 calls it directly (no this adjustment).
// +0xc0 is undeclared (reflection ends at OnActiveEventFlagsChanged 0xa0..0xb0); pointee class unresolved, so raw pointer chase.
// The asm loads a qword at pointee+0x38; the Int return reads only EAX, so the value is the low dword (static_cast<int32> truncation).
// CONFIDENCE: MEDIUM
const uint8* Raw = reinterpret_cast<const uint8*>(this);
const uint8* P = *reinterpret_cast<const uint8* const*>(Raw + 0xc0);
return static_cast<int32>(*reinterpret_cast<const uint64*>(P + 0x38));
