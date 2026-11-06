// CONFIDENCE: HIGH
// Native body 0x141ba8b20; reflected exec 0x1427371f0.
if (OwnerPersistentID == -1)
{
    return nullptr;
}
UWorld* World = GetWorld();
if (!World)
{
    return nullptr;
}
if (AGameStateBase* GameState = World->GetGameState())
{
    const int16 OwnerId = OwnerPersistentID;
    for (APlayerState* PlayerState : GameState->PlayerArray)
    {
        AFortPlayerState* FortState = Cast<AFortPlayerState>(PlayerState);
        if (FortState && FortState->WorldPlayerId == OwnerId)
        {
            return FortState;
        }
    }
}
if (!Cast<AFortGameMode>(World->GetAuthGameMode()))
{
    return nullptr;
}
const int16 OwnerId = OwnerPersistentID;
// Preserve the original second world mode read/cast after the ID snapshot.
AFortGameMode* GameMode = Cast<AFortGameMode>(World->GetAuthGameMode());
for (APlayerState* PlayerState : GameMode->InactivePlayerArray)
{
    AFortPlayerState* FortState = Cast<AFortPlayerState>(PlayerState);
    if (FortState && FortState->CachedPreviousWorldPlayerId == static_cast<int32>(OwnerId))
    {
        return FortState;
    }
}
return nullptr;
