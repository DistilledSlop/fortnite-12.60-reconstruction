// Original body address: 0x144d21040
// Offset evidence: Pointer at object offset 0x140. Own region (lowest own offset 0x30), no reflected member covers it; recovered member NativeRecoveredUint64_0x140 holds the pointer.
// CONFIDENCE: MEDIUM
if (NativeRecoveredUint64_0x140 != 0 && NativeRecoveredUint64_0x140 == reinterpret_cast<uint64>(Quest))
{
    NativeRecoveredUint64_0x140 = 0;
}
