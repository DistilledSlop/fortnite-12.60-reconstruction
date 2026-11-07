// Original body address: 0x145110cf0
// Offset evidence: Byte at 0x66, int32 at 0x58 and int32 at 0x5c. Own region (lowest own offset 0x48), none covered by reflected members; recovered members NativeRecoveredUint8_0x66, NativeRecoveredInt32_0x58, NativeRecoveredInt32_0x5c. Returns true only when byte 0x66 is zero and [0x58] < [0x5c] (signed).
// CONFIDENCE: MEDIUM
return NativeRecoveredUint8_0x66 == 0 && NativeRecoveredInt32_0x58 < NativeRecoveredInt32_0x5c;
