// Original body: 0x1426c8650 (thunk 0x142b4f360)
// Offset evidence: AND dword [RCX+0xdf8],~0x8; OR dword with (DL<<3)
// CONFIDENCE: HIGH
uint32* Flags = reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(this) + 0xdf8);
*Flags = (*Flags & ~0x8u) | (bNewEnabled ? 0x8u : 0u);
