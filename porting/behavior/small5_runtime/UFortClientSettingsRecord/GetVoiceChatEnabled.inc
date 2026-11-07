// Original body address: 0x141e2c560 (thunk 0x14287e6c0)
// Offset evidence: byte [RCX+0x555] = bEnableVoiceChat (reflected bool bit, mask 1); byte [RCX+0x8a8] = NativeRecoveredBool_0x8a8 (unreflected gap in own region).
// Asm: if byte 0x555 == 0 return false; if byte 0x8a8 != 0 return false; return true.
// CONFIDENCE: MEDIUM
return bEnableVoiceChat && !NativeRecoveredBool_0x8a8;
