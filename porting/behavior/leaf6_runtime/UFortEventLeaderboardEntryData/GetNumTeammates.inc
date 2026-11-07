// Behaviour snippet: UFortEventLeaderboardEntryData::GetNumTeammates (body only; spliced into the generated member function).
// Original address: 0x144d386c0 (asm: MOV RAX,[RCX+0x28]; MOV EAX,[RAX+0x78]; RET). Thunk 0x145218980 calls it directly.
// +0x28 pointer undeclared (pointee unresolved). Dword load, int32 return.
// CONFIDENCE: HIGH
const uint8* Raw = reinterpret_cast<const uint8*>(this);
const uint8* P = *reinterpret_cast<const uint8* const*>(Raw + 0x28);
return *reinterpret_cast<const int32*>(P + 0x78);
