// Native helper 0x141e61390 (static helper on UFortMatchmakingContext; callers include GetLobbyBeaconClient body 0x142197110,
// which dereferences the returned pointer at +0x158).
// Asm: MOV RAX,[RCX+0x430]; RET. The asm has no null check; the field has no reflected member, so a raw pointer load is used.
// The pointee class is not identified from the callers, so the return type is UObject*.
// CONFIDENCE: MEDIUM
return *reinterpret_cast<UObject* const*>(reinterpret_cast<const uint8*>(MatchmakingContext) + 0x430);
