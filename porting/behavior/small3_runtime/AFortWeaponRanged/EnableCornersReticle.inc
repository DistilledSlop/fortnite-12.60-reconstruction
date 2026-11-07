// Original body: 0x1426c8670 (thunk 0x142b4f3f0)
// Offset evidence: AND dword [RCX+0xdf8],~0x10; OR dword with (DL<<4)
// CONFIDENCE: HIGH
uint32* Flags = reinterpret_cast<uint32*>(reinterpret_cast<uint8*>(this) + 0xdf8);
*Flags = (*Flags & ~0x10u) | (bNewEnabled ? 0x10u : 0u);
