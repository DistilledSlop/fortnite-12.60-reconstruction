// Behaviour snippet: UFortUICameraManagerComponent::SetCustomCameraRotationOffset (body only; spliced into the generated member function).
// Original body address: 0x141cb79a0. Thunk: 0x142b29a20.
// Evidence (asm): MOVSD [RCX+0x1a0] <- [RDX] (8 bytes); MOV dword [RCX+0x1a8] <- [RDX+8] (4 bytes). Contiguous 12 bytes at +0x1a0..+0x1ab.
// Param type FRotator (12 bytes, reflection struct VA 0x142eeb730). No ReferenceParm flag, so by value (Win64 passes the 12-byte struct by pointer, as the asm reads [RDX]).
// Destination is undeclared storage in this class, so raw offsets.
// CONFIDENCE: HIGH
uint8* Raw = reinterpret_cast<uint8*>(this);
FMemory::Memcpy(Raw + 0x1a0, &Offset, 12);
