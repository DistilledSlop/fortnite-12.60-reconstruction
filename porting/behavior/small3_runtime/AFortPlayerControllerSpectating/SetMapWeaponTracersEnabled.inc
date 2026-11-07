// Original body: 0x1423eb040 (thunk 0x142a6cb80)
// Offset evidence: AND byte [RCX+0x2bf0],0xfe ; OR byte [RCX+0x2bf0],DL
// CONFIDENCE: HIGH
NativeRecoveredUint8_0x2bf0 = static_cast<uint8>((NativeRecoveredUint8_0x2bf0 & 0xfe) | (bEnable ? 1u : 0u));
