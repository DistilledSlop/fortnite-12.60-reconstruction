// CONFIDENCE: high
// 0x14262adc0. The cache is the original 16-bit team mask at +0xcc.
AActor* Owner = GetOwner();
AFortPlayerPawn* ObserverPawn = Cast<AFortPlayerPawn>(Observer);
if (Owner && Observer)
{
    UWorld* OwnerWorld = GEngine->GetWorldFromContextObject(Owner, EGetWorldErrorMode::ReturnNull);
    AFortGameState* OwnerGameState = OwnerWorld ? Cast<AFortGameState>(OwnerWorld->GetGameState()) : nullptr;
    if (OwnerGameState && OwnerGameState->NativeAreFriendlyActors163(Owner, Observer) &&
        (!ObserverPawn || !ObserverPawn->IsDead()))
    {
        return true;
    }
}

AFortPlayerPawn* OwnerPawn = Cast<AFortPlayerPawn>(Owner);
if (OwnerPawn && OwnerPawn->bDamagedEnemy)
{
    return true;
}
UWorld* World = GetWorld();
AFortGameState* GameState = Cast<AFortGameState>(World->GetGameState());
if (GameState && GameState->bPlayersAlwaysVisible && OwnerPawn && ObserverPawn && !ObserverPawn->IsDead())
{
    return true;
}
if (VisibilityBehavior == EFortVisibilityBehavior::AlwaysSeen)
{
    return true;
}

const auto ResolveTeam = [Observer]() -> uint8
{
    IFortTeamActorInterface* TeamActor = Observer ? Cast<IFortTeamActorInterface>(Observer) : nullptr;
    if (!TeamActor && Observer && Observer->GetInstigator())
    {
        TeamActor = Cast<IFortTeamActorInterface>(Observer->GetInstigator());
    }
    return TeamActor ? TeamActor->NativeGetTeam163() : 0;
};
if (VisibilityBehavior == EFortVisibilityBehavior::OnceSeenAlwaysSeen)
{
    const uint32 Bit = uint32(1) << (ResolveTeam() & 31);
    if (bForce && bRevealWithMap)
    {
        NativeCachedTeamBits_0xcc = uint16(NativeCachedTeamBits_0xcc | Bit);
        return true;
    }
    if ((NativeCachedTeamBits_0xcc & Bit) != 0)
    {
        return true;
    }
}

if (Radius >= 0.0f)
{
    FVector ObserverLocation = Observer->GetActorLocation();
    FVector OwnerLocation = Owner->GetActorLocation();
    if (bDistanceCheck2D)
    {
        ObserverLocation.Z = 0.0f;
        OwnerLocation.Z = 0.0f;
    }
    if ((OwnerLocation - ObserverLocation).SizeSquared() > Radius * Radius)
    {
        return false;
    }
}
if (bCheckFacing)
{
    const FVector Forward = Observer->GetActorForwardVector();
    FVector Direction = Owner->GetActorLocation() - Observer->GetActorLocation();
    const float LengthSquared = Direction.SizeSquared();
    if (LengthSquared > SMALL_NUMBER)
    {
        Direction *= FMath::InvSqrt(LengthSquared);
    }
    if (!(FVector::DotProduct(Forward, Direction) >= 0.642f))
    {
        return false;
    }
}
if (bCheckLOS)
{
    FVector ObserverEyes;
    FVector OwnerEyes;
    FRotator EyeRotation;
    Observer->GetActorEyesViewPoint(ObserverEyes, EyeRotation);
    Owner->GetActorEyesViewPoint(OwnerEyes, EyeRotation);
    FHitResult Hit;
    FCollisionQueryParams Params(FName(TEXT("CheckProximityLOS")), true, Observer);
    if (GetWorld()->LineTraceSingleByChannel(Hit, ObserverEyes, OwnerEyes,
        static_cast<ECollisionChannel>(3), Params) && Hit.GetActor() != Owner)
    {
        return false;
    }
}
if (VisibilityBehavior == EFortVisibilityBehavior::OnceSeenAlwaysSeen)
{
    NativeCachedTeamBits_0xcc = uint16(NativeCachedTeamBits_0xcc | (uint32(1) << (ResolveTeam() & 31)));
}
return true;
