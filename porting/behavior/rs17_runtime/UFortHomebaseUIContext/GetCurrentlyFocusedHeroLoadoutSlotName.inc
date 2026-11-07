// Original body address: 0x141132080 (thunk 0x1438aa6b0)
// Offset evidence: MOV RAX,[RCX+0xe0]; MOV [RDX],RAX; MOV RAX,RDX; RET. Returns the FName at object +0xe0 (unreflected gap, see NativeRecoveredUint64_0xe0).
// CONFIDENCE: HIGH
return *reinterpret_cast<const FName*>(&NativeRecoveredUint64_0xe0);
