// helper 0x142ee77f0 (AFortPlayerPawn host): MOV EAX,[RCX+0xc]; INC EAX; MOV [RCX+0xc],EAX; RET.
// Offset 0xc is not covered by a single reflected member of AFortPlayerPawn, so raw int32 access is used.
// CONFIDENCE: MEDIUM
int32* RawCounter = reinterpret_cast<int32*>(reinterpret_cast<uint8*>(Target) + 0xc);
++(*RawCounter);
