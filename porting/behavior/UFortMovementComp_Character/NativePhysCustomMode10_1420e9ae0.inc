// Original address: 0x1420e9ae0
// CONFIDENCE: high
if (deltaTime < 1e-6f || !CharacterOwner || !UpdatedComponent)
{
    return;
}

AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner);
if (!Player)
{
    return;
}

const float MaxTimeStep = FMath::Clamp(SurfaceSwimmingWaterControlParams.WaterSimMaxTimeStep.GetValueAtLevel(0.0f), 0.05f, 1.0f);
const float SubStepTime = FMath::Clamp(SurfaceSwimmingWaterControlParams.WaterSimSubStepTime.GetValueAtLevel(0.0f), 1.0f / 30.0f, MaxTimeStep);

float RemainingTime = deltaTime;
int32 Iter = Iterations;
int32 SubIter = 0;

if (RemainingTime >= 1e-6f)
{
    while (Iter < MaxSimulationIterations)
    {
        Iter++;
        float TimeTick = RemainingTime;
        if (RemainingTime > MaxTimeStep)
        {
            TimeTick = (Iter < MaxSimulationIterations) ? FMath::Min(RemainingTime * 0.5f, MaxTimeStep) : MaxTimeStep;
        }
        else if (RemainingTime > SubStepTime && Iter < MaxSimulationIterations)
        {
            TimeTick = FMath::Min(RemainingTime * 0.5f, SubStepTime);
        }
        RemainingTime -= TimeTick;

        if (Player->JumpForceTimeRemaining > 0.0f)
        {
            Player->JumpForceTimeRemaining -= FMath::Min(TimeTick, Player->JumpForceTimeRemaining);
        }

        if (SubIter > 0)
        {
            UpdateCharacterStateBeforeMovement(TimeTick);
        }

        const FVector OldVelocity = Velocity;
        const bool bWaterJump = Player->bIsWaterJump;
        NativeComputeWaterBobbingAndDrag1420f6570(TimeTick);

        float MaxUpSpeed = SurfaceSwimmingWaterControlParams.MaxSpeedUp.GetValueAtLevel(0.0f);
        const float MaxDownSpeed = SurfaceSwimmingWaterControlParams.MaxSpeedDown.GetValueAtLevel(0.0f);
        if (bWaterJump)
        {
            const FWaterJumpParams& JumpParams = Player->Native_1423320a0() ? WaterJumpParamsSprint : WaterJumpParams;
            const float WaterJumpMaxZ = JumpParams.WaterJumpMaxVelocityZ.GetValueAtLevel(0.0f);
            if (MaxUpSpeed < WaterJumpMaxZ)
            {
                MaxUpSpeed = WaterJumpMaxZ;
            }
        }
        Velocity.Z = FMath::Clamp(Velocity.Z, -FMath::Abs(MaxDownSpeed), FMath::Abs(MaxUpSpeed));

        RestorePreAdditiveRootMotionVelocity();
        if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
        {
            float SwimFriction = (Player->Native_1423320a0()
                ? SurfaceSwimmingWaterControlParams.FrictionSprinting
                : SurfaceSwimmingWaterControlParams.Friction).GetValueAtLevel(0.0f);

            const float DotThresh = FMath::Clamp(SurfaceSwimmingWaterControlParams.FrictionDirectionChangeDot.GetValueAtLevel(0.0f), -1.0f, 1.0f);
            const FVector2D AccelDir2D = FVector2D(Acceleration.X, Acceleration.Y).GetSafeNormal();
            const FVector2D VelDir2D = FVector2D(Velocity.X, Velocity.Y).GetSafeNormal();
            if (FVector2D::DotProduct(VelDir2D, AccelDir2D) < DotThresh)
            {
                SwimFriction *= SurfaceSwimmingWaterControlParams.FrictionDirectionChangeMultiplier.GetValueAtLevel(0.0f);
            }

            const float SavedVelZ = Velocity.Z;
            Velocity.Z = 0.0f;
            const float BrakingDecel = SurfaceSwimmingWaterControlParams.BrakingDeceleration.GetValueAtLevel(0.0f);
            CalcVelocity(TimeTick, SwimFriction, false, BrakingDecel);
            Velocity.Z = SavedVelZ;
        }

        ApplyRootMotionToVelocity(TimeTick);
        bJustTeleported = false;

        const FVector OldLoc = UpdatedComponent->GetComponentLocation();
        const FVector Adjusted = ((OldVelocity + Velocity) * 0.5f) * TimeTick;
        FHitResult Hit(1.0f);
        SafeMoveUpdatedComponent(Adjusted, UpdatedComponent->GetComponentQuat(), true, Hit);

        if (Hit.IsValidBlockingHit())
        {
            HandleImpact(Hit, TimeTick, Adjusted);
            if (MovementMode == MOVE_Custom && CustomMovementMode == 10)
            {
                SlideAlongSurface(Adjusted, 1.0f - Hit.Time, Hit.Normal, Hit, true);
                if (MovementMode == MOVE_Custom && CustomMovementMode == 10)
                {
                    if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && TimeTick > 1e-6f)
                    {
                        Velocity = (UpdatedComponent->GetComponentLocation() - OldLoc) / TimeTick;
                    }
                }
                else
                {
                    StartNewPhysics(RemainingTime, Iter);
                    break;
                }
            }
            else
            {
                StartNewPhysics(RemainingTime, Iter);
                break;
            }
        }

        if (MovementMode != MOVE_Custom || CustomMovementMode != 10)
        {
            StartNewPhysics(RemainingTime, Iter);
            break;
        }

        SubIter++;
        if (RemainingTime < 1e-6f)
        {
            break;
        }
    }
}
