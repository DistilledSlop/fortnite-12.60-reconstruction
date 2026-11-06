// Original address: 0x14283bb70 (exec thunk; body 0x141918950).
// Decompile source: porting/generated/native95-impl/0x141918950.c (thunk: porting/generated/native95-broad/0x14283bb70.c)
// CONFIDENCE: HIGH
// Body: an index outside [0, NativeRecoveredInt_0x7a0) returns 1.0. Otherwise it reads the float at +0xb8 of the
// 0x160-byte spring record at NativeRecoveredPointer_0x798 + SpringIdx * 0x160.
if (SpringIdx < 0 || SpringIdx >= NativeRecoveredInt_0x7a0)
{
    return 1.0f;
}
const uint8* Springs = reinterpret_cast<const uint8*>(NativeRecoveredPointer_0x798);
return *reinterpret_cast<const float*>(Springs + static_cast<int64>(SpringIdx) * 0x160 + 0xb8);
