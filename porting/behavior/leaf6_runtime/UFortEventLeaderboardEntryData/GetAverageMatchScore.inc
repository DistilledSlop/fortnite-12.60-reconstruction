// Behaviour snippet: UFortEventLeaderboardEntryData::GetAverageMatchScore (body only; spliced into the generated member function).
// Original address: 0x144d36de0 (asm: MOV RAX,[RCX+0x28]; MOVD XMM0,[RAX+0x90]; MOVD XMM1,[RAX+0x108]; CVTDQ2PS x2; DIVSS; RET).
// Thunk 0x145218610 calls the body directly (no this adjustment). +0x28 undeclared in header (UObject base is 0x28 bytes); pointee unresolved.
// Signed int32 loads converted to float (CVTDQ2PS), then float division. No null check in asm.
// CONFIDENCE: HIGH
const uint8* Raw = reinterpret_cast<const uint8*>(this);
const uint8* P = *reinterpret_cast<const uint8* const*>(Raw + 0x28);
return static_cast<float>(*reinterpret_cast<const int32*>(P + 0x90)) / static_cast<float>(*reinterpret_cast<const int32*>(P + 0x108));
