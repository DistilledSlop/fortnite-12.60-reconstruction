// CONFIDENCE: high
// CANDIDATE ONLY. Original 0x1420e1a80, virtual +0x3f0.
float Speed = Super::GetMaxSpeed();
float Multiplier = 1.0f;
if (const AFortPawn* Pawn = Cast<AFortPawn>(PawnOwner))
{
    if (!FortMovementSpeed64::Zero(Pawn->PushMomentum)) return Pawn->PushMomentum.Size();
    uint8 Style;
    Speed = Pawn->NativeGetMovementSpeed14231ab10(*Pawn->NativeGetCurrentMovementStyle14231a980(&Style));
    APhysicsVolume* Volume = Pawn->GetPawnPhysicsVolume();
    if (Volume && (Volume->IsA<AWaterVolume_Deep>() || Volume->IsA<AWaterVolume_Shallow>()))
        Multiplier = Volume->FluidFriction;
}
if (NativeAirDirection_1420e4ab0() || NativeAirDirection_1420e4740())
{
    const AFortPlayerPawn* Pawn = Cast<AFortPlayerPawn>(CharacterOwner);
    if (!Pawn || !Pawn->CurrentSkyTube)
        Multiplier *= FortMovementSpeed64::Normal3D(NativeAirAcceleration()).Size2D();
}
if (const AFortPlayerPawn* Pawn = Cast<AFortPlayerPawn>(CharacterOwner))
{
    if (MovementMode == MOVE_Falling)
    {
        if (!Pawn->bIsWaterJump)
        {
            const float Override = Pawn->NativeGetJumpHorizontalVelocityOverride14231a7c0();
            if (Override > 0.0f) return Override * Multiplier;
        }
        else
        {
            const float HorizontalSpeed = Velocity.Size2D();
            Speed = FortMovementSpeed64::MaxSecond(HorizontalSpeed, Speed);
        }
    }
    else if (MovementMode == MOVE_Custom && CustomMovementMode == 10 && Pawn->CurrentSkyTube)
    {
        const FVector Direction = !(FMath::Abs(Velocity.X) > 0.0001f || FMath::Abs(Velocity.Y) > 0.0001f || FMath::Abs(Velocity.Z) > 0.0001f) ?
            UpdatedComponent->GetForwardVector() : FortMovementSpeed64::Normal2D(Velocity);
        const float Projection = FVector::DotProduct(NativeRecoveredFVector_0x1d24, Direction);
        const FVector Adjusted = Direction * Speed + Direction * Projection;
        if (!(FVector::DotProduct(Adjusted, Direction) >= 0.0f)) Speed = 30.0f;
        else
        {
            const float Magnitude = Adjusted.Size();
            Speed = FortMovementSpeed64::MaxSecond(Magnitude, 30.0f);
        }
    }
}
return Multiplier * Speed;
