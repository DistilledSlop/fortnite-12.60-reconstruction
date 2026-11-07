// Original body address: 0x144d83640 (thunk 0x145225a00)
// Offset evidence: byte flags at +0x590, +0x578 and +0x592 (unreflected own-region bytes, member_field NativeRecoveredByte entries). The asm counts the nonzero flags and returns count != 0, i.e. a logical OR.
// CONFIDENCE: HIGH
return NativeRecoveredByte_0x590 != 0 || NativeRecoveredByte_0x578 != 0 || NativeRecoveredByte_0x592 != 0;
