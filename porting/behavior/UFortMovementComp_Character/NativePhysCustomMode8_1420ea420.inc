// Original address: 0x1420ea420
// CONFIDENCE: high
AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner);
if (!Player)
{
    return;
}

float RemainingTime = deltaTime;
int32 Iter = Iterations;
while (RemainingTime >= 1e-6f && Iter < MaxSimulationIterations)
{
    Iter++;
    const float TimeTick = GetSimulationTimeStep(RemainingTime, Iter);
    RemainingTime -= TimeTick;

    RestorePreAdditiveRootMotionVelocity();

    FVector SnapCorrection = FVector::ZeroVector;
    bool bPastEnd = false;
    AFortAthenaZipline* Zipline = Player->ZiplineState.Zipline;
    if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && Zipline && UpdatedComponent)
    {
        const FVector StartPos = Zipline->StartPosition;
        const FVector EndPos = Zipline->EndPosition;
        const FVector Segment = EndPos - StartPos;
        const float SegmentLen = FMath::Max(Segment.Size(), 1e-8f);
        const FVector ZiplineDir = Segment / SegmentLen;

        if (FMath::Abs(CurrentZiplineVelocityDirection) > 1e-8f)
        {
            const FVector HorizDir = FVector(ZiplineDir.X * CurrentZiplineVelocityDirection, ZiplineDir.Y * CurrentZiplineVelocityDirection, 0.0f).GetSafeNormal();
            const FVector InputDir = Acceleration.GetSafeNormal();
            if (FVector::DotProduct(InputDir, HorizDir) < -0.7f)
            {
                CurrentZiplineVelocityDirection = -CurrentZiplineVelocityDirection;
            }
        }
        else
        {
            Velocity = FVector::ZeroVector;
            const FVector Forward = Player->GetActorForwardVector();
            CurrentZiplineVelocityDirection = (FVector::DotProduct(Forward, ZiplineDir) <= 0.0f) ? -1.0f : 1.0f;
            if (const UWorld* World = GetWorld())
            {
                GracePeriodToConnectToZipline = World->GetTimeSeconds();
            }
        }

        const float AccelMag = Zipline->ZiplineAcceleration.GetValueAtLevel(0.0f) * CurrentZiplineVelocityDirection;
        const float MaxSpeed = Zipline->ZiplineVelocity.GetValueAtLevel(0.0f);
        Velocity += ZiplineDir * (AccelMag * TimeTick);
        if (MaxSpeed >= 0.0001f)
        {
            Velocity = Velocity.GetClampedToMaxSize(MaxSpeed);
        }
        else
        {
            Velocity = FVector::ZeroVector;
        }

        const FVector SocketLoc = UpdatedComponent->GetComponentLocation() + Player->ZiplineState.SocketOffset;
        const float DistAlong = FVector::DotProduct(SocketLoc - StartPos, ZiplineDir);
        const FVector ClosestOnLine = StartPos + ZiplineDir * DistAlong;
        SnapCorrection = ClosestOnLine - SocketLoc;
        if ((CurrentZiplineVelocityDirection < 0.0f && DistAlong < 0.0f) ||
            (CurrentZiplineVelocityDirection > 0.0f && DistAlong > SegmentLen))
        {
            bPastEnd = true;
        }
    }

    ApplyRootMotionToVelocity(TimeTick);
    bJustTeleported = false;

    if (!UpdatedComponent)
    {
        return;
    }

    const UWorld* World = GetWorld();
    const bool bSweep = World ? ((World->GetTimeSeconds() - GracePeriodToConnectToZipline) >= 0.5f) : true;
    const FVector MoveDelta = Velocity * TimeTick + SnapCorrection;
    FHitResult Hit(1.0f);
    SafeMoveUpdatedComponent(MoveDelta, UpdatedComponent->GetComponentQuat(), bSweep, Hit);

    bool bHitSolid = (Hit.Time < 1.0f);
    if (bHitSolid)
    {
        if (const ABuildingSMActor* Building = Cast<ABuildingSMActor>(Hit.GetActor()))
        {
            if (Building->CanBeDamaged())
            {
                bHitSolid = false;
            }
        }
    }

    if (bHitSolid || bPastEnd)
    {
        if (Player->GetLocalRole() == ROLE_AutonomousProxy || Player->GetLocalRole() == ROLE_Authority)
        {
            Player->NativeEndZiplining142314600(false);
        }
    }

    if (!bJustTeleported && !HasAnimRootMotion())
    {
        CurrentRootMotion.HasOverrideVelocity();
    }
}
