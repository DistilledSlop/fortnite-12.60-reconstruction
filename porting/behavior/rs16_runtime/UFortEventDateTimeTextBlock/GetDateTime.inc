// Original body address: 0x1407c25a0 (thunk 0x14080ffa0)
// Offset evidence: the class has no reflected own properties; the 8-byte FDateTime at +0x2d8 is copied as-is (MOV RAX,[RCX+0x2d8]; MOV [RDX],RAX). Read raw.
// CONFIDENCE: HIGH
const uint8* Raw = reinterpret_cast<const uint8*>(this);
return *reinterpret_cast<const FDateTime*>(Raw + 0x2d8);
