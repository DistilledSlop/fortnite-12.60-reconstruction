// Original body address: 0x144e7d220
// Offset evidence: MOVZX EAX,byte ptr [RCX+0x4c8]; MOV byte ptr [RDX],AL. Offset 0x4c8 is not covered by a reflected member; SwipeDetector (struct at 0x440) has no recorded size, so the byte is read raw.
// CONFIDENCE: MEDIUM
const uint8* Raw = reinterpret_cast<const uint8*>(this);
bEnabled = *reinterpret_cast<const bool*>(Raw + 0x4c8);
