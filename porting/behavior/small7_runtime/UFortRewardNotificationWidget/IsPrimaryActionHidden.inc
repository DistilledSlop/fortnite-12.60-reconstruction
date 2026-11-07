// Original body address: 0x144e7d230
// Offset evidence: MOVZX EAX,byte ptr [RCX+0x4c9]; MOV byte ptr [RDX],AL. Offset 0x4c9 read raw (same reason as 0x4c8).
// CONFIDENCE: MEDIUM
const uint8* Raw = reinterpret_cast<const uint8*>(this);
bHidden = *reinterpret_cast<const bool*>(Raw + 0x4c9);
