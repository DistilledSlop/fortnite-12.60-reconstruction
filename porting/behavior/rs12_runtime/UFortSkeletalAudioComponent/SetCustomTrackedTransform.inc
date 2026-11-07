// Original body 0x1424bbde0 (thunk 0x142ada910). Offset evidence: BoneInstances TArray at +0xc8 (Data) and +0xd0 (Num, reflected); element stride 0xc0 (LEA x3, SHL 6). Element fields from asm: +0x50 FQuat previous rotation, +0x60 FQuat rotation, +0x98 FVector previous translation, +0xa4 FVector translation. Reflected FSkeletalAudioBoneInstance is smaller than the stride, so raw element access is used.
// Index < 0 or Index >= Num returns. Old rotation and translation are copied into the previous slots before the new transform is written.
// CONFIDENCE: MEDIUM
if (Index < 0 || Index >= BoneInstances.Num())
{
    return;
}
uint8* Elem = reinterpret_cast<uint8*>(BoneInstances.GetData()) + static_cast<int64>(Index) * 192;
*reinterpret_cast<FQuat*>(Elem + 0x50) = *reinterpret_cast<const FQuat*>(Elem + 0x60);
*reinterpret_cast<FVector*>(Elem + 0x98) = *reinterpret_cast<const FVector*>(Elem + 0xa4);
*reinterpret_cast<FQuat*>(Elem + 0x60) = Transform.Rotation;
*reinterpret_cast<FVector*>(Elem + 0xa4) = Transform.Translation;
