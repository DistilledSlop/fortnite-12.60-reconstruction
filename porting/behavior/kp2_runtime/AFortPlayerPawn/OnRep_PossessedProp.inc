// Behaviour snippet: AFortPlayerPawn::OnRep_PossessedProp (body only; spliced into the generated member function).
// Original body address: 0x14233d310 (thunk 0x142a7e0c0)
// Offset evidence: this+0x15d8 = OnPossessedProp (InlineMulticastDelegate), this+0x15e8 = PossessedProp (Object), this+0x278 = Mesh (ACharacter).
// Anim instance +0x340 = UFortAnimInstance::bEnableAdditiveLayer (bool_bits.json, byte 0x340 mask 0x1); set when PossessedProp is null, cleared otherwise.
// Callee evidence: 0x1406a9dc0 = OnPossessedProp.Broadcast(this, PossessedProp); 0x144114bd0 = AFortPawn::NativeGetMeshAnimInstance144114bd0; 0x1427b4d30 = UFortAnimInstance::StaticClass() IsA check.
// CONFIDENCE: MEDIUM
static_assert(sizeof(UFortAnimInstance) > 0, "UFortAnimInstance must be complete");
OnPossessedProp.Broadcast(this, PossessedProp);
if (Mesh != nullptr)
{
    UAnimInstance* AnimInstance = AFortPawn::NativeGetMeshAnimInstance144114bd0(Mesh);
    UFortAnimInstance* FortAnimInstance = Cast<UFortAnimInstance>(AnimInstance);
    if (FortAnimInstance != nullptr)
    {
        FortAnimInstance->bEnableAdditiveLayer = (PossessedProp == nullptr);
    }
}
