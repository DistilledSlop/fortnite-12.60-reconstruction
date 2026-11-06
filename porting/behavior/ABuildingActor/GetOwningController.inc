// CONFIDENCE: HIGH
// Native body 0x141ba8530; reflected exec 0x1427371c0.
const int16 OwnerId = OwnerPersistentID;
if (OwnerId == -1)
{
    return nullptr;
}
UWorld* World = GetWorld();
if (!World || GetLocalRole() != ROLE_Authority)
{
    return nullptr;
}
for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
{
    AFortPlayerController* Controller = Cast<AFortPlayerController>(It->Get());
    if (Controller)
    {
        const AFortPlayerState* State = Cast<AFortPlayerState>(Controller->PlayerState);
        const int16 WorldId = State ? State->WorldPlayerId : -1;
        if (WorldId == OwnerId)
        {
            return Controller;
        }
    }
}
return nullptr;
