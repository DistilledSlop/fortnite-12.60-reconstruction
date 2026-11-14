// CONFIDENCE: HIGH
// 0x141e5b710. UFortGameInstance member; proposed signature:
// void NativeGatherWorldProfiles(TArray<UFortMcpProfileWorld*>& OutProfiles, bool bInitialPlayersOnly)
// The false branch assumes each registered player is non-null, as the original does.
OutProfiles.Empty();
for (UFortRegisteredPlayerInfo* Player : RegisteredPlayers)
{
    if (!bInitialPlayersOnly ||
        (Player && !Player->IsUnreachable() && !Player->IsPendingKill() && Player->bIsInitialPlayer))
    {
        if (Player->WorldProfile)
        {
            OutProfiles.Add(Player->WorldProfile);
        }
    }
}
