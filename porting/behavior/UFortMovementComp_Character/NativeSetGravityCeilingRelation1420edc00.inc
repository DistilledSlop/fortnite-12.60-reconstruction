// Original address: 0x1420edc00
// CONFIDENCE: high
const int32 Clamped = FMath::Clamp(NewRelation, -1, 1);
if (GravityCeilingRelation != Clamped)
{
    GravityCeilingRelation = Clamped;
    if (AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner))
    {
        Player->OnBallooningGravityCeilingChanged.Broadcast(GravityCeilingRelation);
    }
}
