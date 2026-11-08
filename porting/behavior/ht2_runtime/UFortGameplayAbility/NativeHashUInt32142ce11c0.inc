// Helper 0x142ce11c0, host UFortGameplayAbility (static). 11 instructions, RET-terminated, no callees.
// Asm: MOVZX EAX,CX; MOV EDX,ECX; MOV ECX,EAX; SHR EDX,0x10; IMUL R8D,EDX,0x80001; SHL ECX,0x10;
//      ADD R8D,EAX; SHR EAX,0x4; ADD R8D,ECX; ADD EAX,R8D; RET
// Evidence: caller ABuildingRift::StartEnvironmentQuery (0x141c1f6e0) loads ECX from the low dword of a qword before the call.
// CONFIDENCE: HIGH
const uint32 LowHalf = Value & 0xFFFFu;
const uint32 HighHalf = Value >> 16;
uint32 Mixed = HighHalf * 0x80001u;
Mixed += LowHalf;
Mixed += LowHalf << 16;
return (LowHalf >> 4) + Mixed;
