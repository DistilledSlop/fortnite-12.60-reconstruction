// Original body 0x14233a020 (thunk 0x142a7dd90; same body as OnRep_GroupEmoteLookTarget).
// Offset evidence: [RCX+0x1108] mask 0x4 = bIsEmoteLeader (bool_bits); [RCX+0x1110] = GroupEmoteLookTarget;
// AActor::RootComponent at +0x130 of both pawns; component location = 3 floats at +0x1d0 (raw, engine USceneComponent);
// 0x1134 float, 0x1138 float (Yaw of rotator), 0x113c int32 are unreflected gap members of AFortPlayerPawn.
// Zero fallback vectors from writable snapshot 0x148218630/0x148218638 (0.0) are FVector::ZeroVector.
// CONFIDENCE: MEDIUM
static_assert(sizeof(UFortKismetLibrary) > 0, "UFortKismetLibrary declares the vector-to-rotator helper");
if (!bIsEmoteLeader)
{
    return;
}
AFortPlayerPawn* EmoteTarget = GroupEmoteLookTarget;
if (!EmoteTarget)
{
    return;
}
USceneComponent* TargetRoot = EmoteTarget->RootComponent;
FVector TargetLoc = TargetRoot
    ? *reinterpret_cast<const FVector*>(reinterpret_cast<const uint8*>(TargetRoot) + 0x1d0)
    : FVector::ZeroVector;
USceneComponent* SelfRoot = RootComponent;
FVector SelfLoc = SelfRoot
    ? *reinterpret_cast<const FVector*>(reinterpret_cast<const uint8*>(SelfRoot) + 0x1d0)
    : FVector::ZeroVector;
FVector DirVector(TargetLoc.X - SelfLoc.X, TargetLoc.Y - SelfLoc.Y, TargetLoc.Z - SelfLoc.Z);
FRotator OutRotator = FRotator::ZeroRotator;
FRotator* DirRotator = UFortKismetLibrary::NativeVectorToRotatorDegrees142c5cdf0(&DirVector, &OutRotator);
// The asm stores the 8-byte pair (Pitch, Yaw) then overwrites the low dword with zero, so only Yaw survives.
NativeRecoveredFloat_0x1134 = 0.0f;
NativeRecoveredFloat_0x1138 = DirRotator->Yaw;
NativeRecoveredInt32_0x113c = 0;
