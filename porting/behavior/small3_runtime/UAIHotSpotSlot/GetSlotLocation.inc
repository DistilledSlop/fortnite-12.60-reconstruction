// Original body: 0x14165f120 (thunk 0x142710d00)
// Offset evidence: MOVUPS XMM1,[RCX+0xa0]; stores x,y,z to hidden return slot
// CONFIDENCE: HIGH
return FVector(NativeRecoveredFloat_0xa0, NativeRecoveredFloat_0xa4, NativeRecoveredFloat_0xa8);
