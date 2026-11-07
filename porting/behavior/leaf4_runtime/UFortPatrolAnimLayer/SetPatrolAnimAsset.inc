// Behaviour snippet: UFortPatrolAnimLayer::SetPatrolAnimAsset (body only; spliced into the generated member function).
// Original address: 0x141881210 (thunk 0x142a202c0).
// asm: MOV [+0x2e8],RDX (NewAnimAsset). CurrentAnimSet (FFortAnimInput_PatrolAnimSet) at +0x2a0 is 0x40 bytes;
// +0x2e8 is the 8-byte slot before CurrentSpeedInterpSpeed (+0x2f0).
// CONFIDENCE: HIGH
NativeRecoveredObject_0x2e8 = reinterpret_cast<uint64>(NewAnimAsset);
