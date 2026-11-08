// Original address: 0x140d91b30 (UFortPartyContext helper; asm: LEA RAX,[RCX+0xf0]; RET)
// Offset evidence: returns the address this+0xf0 with no loads. Callers 0x142250720 and 0x1419a81d0 use the result as a base
// and read the byte at base+0x40 (object +0x130). Reflection names +0xf0 as OnPartyPrivacyChanged; the helper returns the raw address either way.
// CONFIDENCE: MEDIUM
return reinterpret_cast<uint8*>(PartyContextObj) + 0xf0;
