// Original body address: 0x1450e9ce0
// Offset evidence: CMP byte ptr [RCX+0x2f5],0; nonzero returns -1; else dword at 0x2e0. Read raw: reflected Perk (FFortUIPerk at 0x260) has no recorded size, so 0x2e0/0x2f5 may fall inside it.
// CONFIDENCE: MEDIUM
const uint8* Raw = reinterpret_cast<const uint8*>(this);
if (Raw[0x2f5] != 0) { return -1; }
return *reinterpret_cast<const int32*>(Raw + 0x2e0);
