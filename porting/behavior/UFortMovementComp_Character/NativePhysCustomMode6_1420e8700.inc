// Original address: 0x1420e8700
// CONFIDENCE: high
float RemainingTime = deltaTime;
int32 Iter = Iterations;
while (RemainingTime >= 1e-6f && Iter < MaxSimulationIterations)
{
    Iter++;
    const float TimeTick = GetSimulationTimeStep(RemainingTime, Iter);
    RemainingTime -= TimeTick;

    RestorePreAdditiveRootMotionVelocity();
    if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
    {
        if (AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner))
        {
            if (UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent())
            {
                if (const UFortJumpBoostSet* BoostSet = ASC->GetSet<UFortJumpBoostSet>())
                {
                    const float MaxLat = BoostSet->LateralThrust.GetCurrentValue();
                    FVector2D HorizVel(Velocity.X + Acceleration.X * TimeTick, Velocity.Y + Acceleration.Y * TimeTick);
                    if (MaxLat >= 0.0001f)
                    {
                        HorizVel = FVector2D(FVector(HorizVel, 0.0f).GetClampedToMaxSize(MaxLat));
                    }
                    else
                    {
                        HorizVel = FVector2D::ZeroVector;
                    }

                    const float LocZ = UpdatedComponent ? UpdatedComponent->GetComponentLocation().Z : 0.0f;
                    const float AltAlpha = FMath::Clamp((LocZ - 20000.0f) / (25000.0f - 20000.0f), 0.0f, 1.0f);
                    const float UpAccel = BoostSet->UpwardAcceleration.GetCurrentValue();
                    const float MaxUp = BoostSet->UpwardThrust.GetCurrentValue();
                    float NewZ = Velocity.Z + (UpAccel * (-2.0f * AltAlpha + 1.0f)) * TimeTick;
                    if (MaxUp >= 0.0001f)
                    {
                        NewZ = FMath::Clamp(NewZ, -MaxUp, MaxUp);
                    }
                    else
                    {
                        NewZ = 0.0f;
                    }
                    Velocity = FVector(HorizVel.X, HorizVel.Y, NewZ);
                }
            }
        }
    }

    ApplyRootMotionToVelocity(TimeTick);
    bJustTeleported = false;

    if (!UpdatedComponent)
    {
        return;
    }

    const FVector OldLoc = UpdatedComponent->GetComponentLocation();
    const FVector Adjusted = Velocity * TimeTick;
    FHitResult Hit(1.0f);
    SafeMoveUpdatedComponent(Adjusted, UpdatedComponent->GetComponentQuat(), true, Hit);

    if (Hit.Time < 1.0f)
    {
        const FVector VelDir = Velocity.GetSafeNormal();
        if (FMath::Abs(Hit.Normal.Z) < 0.2f && -VelDir.Z < 0.5f && -VelDir.Z > -0.2f && CanStepUp(Hit))
        {
            const FVector StepDelta = Adjusted * (1.0f - Hit.Time);
            if (StepUp(FVector(0.0f, 0.0f, -1.0f), StepDelta, Hit))
            {
                if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
                {
                    Velocity = (UpdatedComponent->GetComponentLocation() - OldLoc) / TimeTick;
                }
                continue;
            }
        }
        HandleImpact(Hit, TimeTick, Adjusted);
        SlideAlongSurface(Adjusted, 1.0f - Hit.Time, Hit.Normal, Hit, true);
    }

    if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
    {
        Velocity = (UpdatedComponent->GetComponentLocation() - OldLoc) / TimeTick;
    }
}
