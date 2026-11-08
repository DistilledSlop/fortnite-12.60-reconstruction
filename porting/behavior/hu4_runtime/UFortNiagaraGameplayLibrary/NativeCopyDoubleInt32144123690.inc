// Helper 0x144123690 (UFortNiagaraGameplayLibrary static). Copies a {double, int32} pair from Source into Target.
// Asm evidence: MOVSD XMM0,qword ptr [RDX] (Source+0); MOV EAX,dword ptr [RDX+0x8] (Source+8);
// MOVSD qword ptr [RCX+0x134],XMM0 (Target+0x134); MOV dword ptr [RCX+0x13c],EAX (Target+0x13c); RET.
// The owning object type is not justified by the job's callers, so Target is raw bytes and no reflected member is used.
// CONFIDENCE: MEDIUM
*reinterpret_cast<double*>(Target + 0x134) = *reinterpret_cast<const double*>(Source);
*reinterpret_cast<int32*>(Target + 0x13c) = *reinterpret_cast<const int32*>(Source + 8);
