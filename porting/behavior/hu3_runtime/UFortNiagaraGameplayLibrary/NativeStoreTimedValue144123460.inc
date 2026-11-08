// Helper 0x144123460, host UFortNiagaraGameplayLibrary (14 callers; same callers as 0x144123220).
// Asm: MOVSD XMM0,[RDX]; MOV EAX,[RDX+8]; MOVSD [RCX+0x128],XMM0; MOV [RCX+0x130],EAX; RET.
// Offset evidence: no reflected member at 0x128/0x130 (lookup_offset returns none); raw access used.
// CONFIDENCE: HIGH
*reinterpret_cast<double*>(reinterpret_cast<uint8*>(Target) + 0x128) = *reinterpret_cast<const double*>(Source);
*reinterpret_cast<int32*>(reinterpret_cast<uint8*>(Target) + 0x130) = *reinterpret_cast<const int32*>(Source + 0x8);
