// Original body address: 0x144e7d200 (body shared with IsPrimaryActionHidden on this class)
// Offset evidence: MOV RAX,[RCX+0x278] (pointer; offset 0x278 is past OnTransitionOutComplete which ends at 0x278; added as NativeRecoveredUint64_0x278). MOVZX byte at pointee+0x4c8 stored to bEnabled. Pointee read raw.
// CONFIDENCE: MEDIUM
const uint8* Target = reinterpret_cast<const uint8*>(NativeRecoveredUint64_0x278);
bEnabled = *reinterpret_cast<const bool*>(Target + 0x4c8);
