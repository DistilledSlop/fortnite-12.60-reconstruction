// Original address: 0x1420e9390
// CONFIDENCE: high
if (deltaTime < 1e-6f)
{
    return;
}

AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner);
AFortPlayerControllerAthena* PC = Player ? Cast<AFortPlayerControllerAthena>(Player->GetController()) : nullptr;
AFortPlayerPawn* LeaderPawn = nullptr;
UFortMovementComp_Character* LeaderMoveComp = nullptr;

if (PC && PC->SkydiveLeader)
{
    LeaderPawn = PC->SkydiveLeader->GetCurrentPawn();
    if (LeaderPawn)
    {
        LeaderMoveComp = Cast<UFortMovementComp_Character>(LeaderPawn->GetMovementComponent());
    }
}

if (!Player || !PC || !LeaderPawn)
{
    const uint8 SavedAirFlag = NativeAirborneFlag;
    NativeAirborneFlag = 1;
    if ((NativePackedAccelFlags_1e38 & 8) == 0 && Player && Player->GetLocalRole() != ROLE_SimulatedProxy)
    {
        CustomFallingAcceleration = Acceleration;
    }
    else
    {
        CustomFallingAcceleration = ReplicatedAcceleration;
    }
    NativeAirborneVelocity_1e0c = Velocity;
    PhysFalling(deltaTime, Iterations);
    NativeAirborneFlag = SavedAirFlag;
    return;
}

const FVector LeaderLoc = LeaderPawn->GetActorLocation();
const FVector OffsetTarget = LeaderLoc + FVector(-500.0f, 0.0f, 0.0f);
float RemainingTime = deltaTime;
int32 Iter = Iterations;
while (RemainingTime >= 1e-6f && Iter < MaxSimulationIterations)
{
    Iter++;
    const float TimeTick = GetSimulationTimeStep(RemainingTime, Iter);
    RemainingTime -= TimeTick;
    if (UpdatedComponent)
    {
        const FVector OldLoc = UpdatedComponent->GetComponentLocation();
        const FVector MoveDelta = OffsetTarget - OldLoc;
        FHitResult Hit(1.0f);
        SafeMoveUpdatedComponent(MoveDelta, UpdatedComponent->GetComponentQuat(), true, Hit);
        const FVector NewLoc = UpdatedComponent->GetComponentLocation();
        if (TimeTick > 1e-6f)
        {
            Velocity = (NewLoc - OldLoc) / TimeTick;
        }
    }
}

const uint8 SavedAirFlag = NativeAirborneFlag;
NativeAirborneFlag = 1;
if (LeaderMoveComp)
{
    CustomFallingAcceleration = (LeaderPawn->GetLocalRole() == ROLE_SimulatedProxy)
        ? LeaderMoveComp->ReplicatedAcceleration
        : LeaderMoveComp->GetCurrentAcceleration();
    NativeAirborneVelocity_1e0c = LeaderMoveComp->Velocity;
}
NativeAirborneFlag = SavedAirFlag;
