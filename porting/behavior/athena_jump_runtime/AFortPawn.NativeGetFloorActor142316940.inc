// CONFIDENCE: HIGH. Complete original body; class lifecycle acceptance remains separate.
// CONFIDENCE: HIGH (complete original0x142316940 control flow, cached engine calls + raw assembly).
// AFortPawn::virtual AActor* NativeGetFloorActor142316940(UPrimitiveComponent** OutComponent)
// Includes: Components/CapsuleComponent.h, GameFramework/CharacterMovementComponent.h,
// Engine/World.h, CollisionQueryParams.h, WorldCollision.h (FCollisionShape).
UCharacterMovementComponent* Movement = GetCharacterMovement();
if (Movement->MovementMode == MOVE_Walking)
{
    if (OutComponent) *OutComponent = GetMovementBase();
    return APawn::GetMovementBaseActor(this);
}
if (!CachedNavFloor)
{
    // Original14406cf40 uses exactly trace tag, false complex, ignore-this and defaults.
    const FCollisionQueryParams Params(SCENE_QUERY_STAT(FloorActorCheck), false, this);
    const FCollisionShape Shape = GetCapsuleComponent()->GetCollisionShape(0.0f);
    USceneComponent* Updated = Movement->UpdatedComponent;
    FVector Bottom;
    if (Updated)
    {
        Bottom = Updated->GetComponentLocation();
        Bottom.Z -= Updated->Bounds.BoxExtent.Z;
    }
    else
    {
        Bottom = FVector(FLT_MAX, FLT_MAX, FLT_MAX); // Original1478de528, explicit original constant.
    }
    const FVector Start(Bottom.X, Bottom.Y, Bottom.Z + 60.0f);
    const FVector End(Bottom.X, Bottom.Y, Bottom.Z - 60.0f);
    const USceneComponent* Root = GetRootComponent();
    const FQuat Rotation = Root ? Root->GetComponentQuat() : FQuat::Identity;
    FHitResult Hit;
    GetWorld()->SweepSingleByChannel(Hit, Start, End, Rotation, ECC_GameTraceChannel11,
        Shape, Params, FCollisionResponseParams::DefaultResponseParam);
    // Original checks the blocking bit, not returned Sweep bool or IsValidBlockingHit().
    CachedNavFloor = Hit.bBlockingHit ? Hit.Component.Get() : nullptr;
}
if (OutComponent) *OutComponent = CachedNavFloor;
return CachedNavFloor ? CachedNavFloor->GetOwner() : nullptr;
