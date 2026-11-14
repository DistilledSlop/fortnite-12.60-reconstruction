// Original address: 0x1420e9010
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
        FVector MoveDir = FVector::ZeroVector;
        if (AFortRemoteControlledPawnAthena* RCPawn = Cast<AFortRemoteControlledPawnAthena>(CharacterOwner))
        {
            if (!RCPawn->bTriggeredForDestroy)
            {
                if (RCPawn->Controller)
                {
                    MoveDir = RCPawn->GetControlRotation().Vector();
                }
                else
                {
                    MoveDir = RCPawn->GetActorForwardVector();
                }
            }
        }
        Velocity = MoveDir * GetMaxSpeed();
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

    if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
    {
        Velocity = (UpdatedComponent->GetComponentLocation() - OldLoc) / TimeTick;
    }
}
