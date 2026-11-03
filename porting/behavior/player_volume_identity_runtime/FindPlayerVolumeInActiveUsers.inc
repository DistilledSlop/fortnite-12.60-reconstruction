// CONFIDENCE: HIGH
// 0x141c7d270 receives &VolumeActivePlayers; the assigned binding is on its owner.
if (!PlayerState)
{
    return nullptr;
}
if (PlayerState->GetUniqueId().IsValid())
{
    for (const FVolumePlayerStateInfo& Entry : VolumeActivePlayers.Items)
    {
        if (Entry.UniquePlayerId == PlayerState->GetUniqueId())
        {
            return Entry.Volume;
        }
    }
    return nullptr;
}

AFortPawn* Pawn = Cast<AFortPawn>(PlayerState->GetPawn());
if (Pawn)
{
    AFortVolumeManager* PawnManager = NativeResolveVolumeManager(Pawn);
    if (PawnManager)
    {
        return PawnManager->NativeFindVolumeAtLocation(Pawn->GetActorLocation());
    }
}
return nullptr;
