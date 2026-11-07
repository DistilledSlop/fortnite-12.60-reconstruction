// Behaviour snippet: UFortPortalHUDDetails::SetCachedWorldPos (body only; spliced into the generated member function).
// Original body address: 0x144ddd980. Thunk: 0x1452b1b40.
// Evidence (asm): MOVSD [RCX+0x2e4] <- [RDX] (8 bytes); MOV dword [RCX+0x2ec] <- [RDX+8] (4 bytes); MOV byte [RCX+0x2e0] <- 1.
// Param type FVector (12 bytes, reflection struct VA 0x142eea590, the getter that FVector_NetQuantize and FPlane derive from). Passed by value (no ReferenceParm flag).
// +0x2e0 is the existing NativeRecoveredByte_0x2e0 (set to 1 here); +0x2e4..+0x2ef is undeclared, so raw offsets.
// CONFIDENCE: HIGH
uint8* Raw = reinterpret_cast<uint8*>(this);
FMemory::Memcpy(Raw + 0x2e4, &InWorldPos, 12);
Raw[0x2e0] = 1;
