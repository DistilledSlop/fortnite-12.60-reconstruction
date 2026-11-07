// Original body: 0x1426c8630 (thunk 0x142b4f2d0)
// Offset evidence: AND dword [RCX+0xdf8],~0x4; OR dword with (DL<<2)
// CONFIDENCE: HIGH
uint32* Flags = reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(this) + 0xdf8);
*Flags = (*Flags & ~0x4u) | (bNewEnabled ? 0x4u : 0u);
