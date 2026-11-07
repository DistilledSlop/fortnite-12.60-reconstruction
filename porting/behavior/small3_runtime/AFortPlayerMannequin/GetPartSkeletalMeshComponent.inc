// Original body: 0x141dcc7e0 (thunk 0x142a7c520)
// Offset evidence: CMP EDX,6; JZ -> XOR EAX,EAX; else qword [RCX+RAX*8+0x408]
// CONFIDENCE: MEDIUM
switch (static_cast<int32>(Part))
{
case 0:
    return reinterpret_cast<USkeletalMeshComponent*>(NativeRecoveredPtr_0x408);
case 1:
    return reinterpret_cast<USkeletalMeshComponent*>(NativeRecoveredPtr_0x410);
case 2:
    return reinterpret_cast<USkeletalMeshComponent*>(NativeRecoveredPtr_0x418);
case 3:
    return reinterpret_cast<USkeletalMeshComponent*>(NativeRecoveredPtr_0x420);
case 4:
    return reinterpret_cast<USkeletalMeshComponent*>(NativeRecoveredPtr_0x428);
case 5:
    return reinterpret_cast<USkeletalMeshComponent*>(NativeRecoveredPtr_0x430);
default:
    return nullptr;
}
