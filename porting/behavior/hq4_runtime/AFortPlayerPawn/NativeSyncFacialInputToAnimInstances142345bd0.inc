// Helper 0x142345bd0 (AFortPlayerPawn static, 2 callers: 0x14234c9d0 tail-jump, 0x1423537e0 call).
// Pawn offsets: +0x18d0 = CharacterPartSkeletalMeshComponents[0], +0x278 = Mesh (ACharacter),
// +0x2720 = double FacialWeight, +0x2728 = int32 FacialMode.
// Writes the facial input into the head-part anim instance (+0x418 double, +0x420 int32) when the part-0
// anim instance IsA UHeadPartAnimInstance, and into the frontend anim instance (+0x2d8 double, +0x2e0 int32)
// when the mesh anim instance IsA UFrontendAnimInstance.
// CONFIDENCE: MEDIUM
UObject* PartComponent = *reinterpret_cast<UObject* const*>(reinterpret_cast<const uint8*>(Pawn) + 0x18D0);
if (PartComponent != nullptr)
{
	UAnimInstance* PartAnim = NativeGetMeshAnimInstance144114bd0(reinterpret_cast<USkeletalMeshComponent*>(PartComponent));
	if (PartAnim != nullptr && PartAnim->IsA(UHeadPartAnimInstance::StaticClass()))
	{
		uint8* HeadPartBase = reinterpret_cast<uint8*>(PartAnim);
		*reinterpret_cast<double*>(HeadPartBase + 0x418) = *reinterpret_cast<const double*>(reinterpret_cast<const uint8*>(Pawn) + 0x2720);
		*reinterpret_cast<int32*>(HeadPartBase + 0x420) = *reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(Pawn) + 0x2728);
	}
}

USkeletalMeshComponent* MeshComponent = Pawn->GetMesh();
if (MeshComponent != nullptr)
{
	UAnimInstance* FrontendAnim = NativeGetMeshAnimInstance144114bd0(MeshComponent);
	if (FrontendAnim != nullptr && FrontendAnim->IsA(UFrontendAnimInstance::StaticClass()))
	{
		uint8* FrontendBase = reinterpret_cast<uint8*>(FrontendAnim);
		*reinterpret_cast<double*>(FrontendBase + 0x2D8) = *reinterpret_cast<const double*>(reinterpret_cast<const uint8*>(Pawn) + 0x2720);
		*reinterpret_cast<int32*>(FrontendBase + 0x2E0) = *reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(Pawn) + 0x2728);
	}
}
