// Original address: 0x141291580 (AAIHotSpot helper; asm: MOVZX EAX,byte ptr [RCX+0x84]; RET)
// Offset evidence: raw byte at object +0x84; no reflected member covers 0x84. Returned unchanged (callers test AL).
// CONFIDENCE: HIGH
return *(reinterpret_cast<const uint8*>(HotSpotObj) + 0x84);
