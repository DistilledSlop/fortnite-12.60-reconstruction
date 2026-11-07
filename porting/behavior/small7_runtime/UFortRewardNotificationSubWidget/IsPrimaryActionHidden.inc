// Original body address: 0x144e7d200 (body shared with IsPrimaryActionEnabled on this class)
// Offset evidence: MOV RAX,[RCX+0x278] (pointer, added as NativeRecoveredUint64_0x278). MOVZX byte at pointee+0x4c8 stored to bHidden. The asm reads the same byte 0x4c8 as IsPrimaryActionEnabled; kept as-is.
// CONFIDENCE: MEDIUM
const uint8* Target = reinterpret_cast<const uint8*>(NativeRecoveredUint64_0x278);
bHidden = *reinterpret_cast<const bool*>(Target + 0x4c8);
