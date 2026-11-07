// Original body: 0x141f66030 (thunk 0x14076d570)
// Offset evidence: CMP [0xe12],0; JZ copy; CMP [0xe13],0; JNZ ret; copy: dword [0x224]=[0x220]
// CONFIDENCE: HIGH
if (NativeRecoveredBool_0xe12 && NativeRecoveredBool_0xe13)
{
    return;
}
TimeOfDayReplicated = TimeOfDay;
