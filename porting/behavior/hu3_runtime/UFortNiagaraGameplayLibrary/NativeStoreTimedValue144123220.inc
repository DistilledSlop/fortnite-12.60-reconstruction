// Helper 0x144123220, host UFortNiagaraGameplayLibrary (14 callers; caller 0x14422ffb0 passes object and &{double,int32}).
// Asm: MOVSD XMM0,[RDX]; MOV EAX,[RDX+8]; MOVSD [RCX+0x11c],XMM0; MOV [RCX+0x124],EAX; RET.
// Offset evidence: no reflected member at 0x11c/0x124 (lookup_offset returns none); raw access used.
// CONFIDENCE: HIGH
*reinterpret_cast<double*>(reinterpret_cast<uint8*>(Target) + 0x11c) = *reinterpret_cast<const double*>(Source);
*reinterpret_cast<int32*>(reinterpret_cast<uint8*>(Target) + 0x124) = *reinterpret_cast<const int32*>(Source + 0x8);
