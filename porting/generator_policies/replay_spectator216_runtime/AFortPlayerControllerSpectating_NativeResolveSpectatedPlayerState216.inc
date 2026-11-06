// CONFIDENCE: high
// 0x1423d0d80; original captures and storage proof in native-evidence.json.
if (!Id.IsValid())
{
    return nullptr;
}
UWorld* World = GetWorld();
AGameStateBase* State = World ? World->GetGameState() : nullptr;
if (!State)
{
    return nullptr;
}
for (APlayerState* Candidate : State->PlayerArray)
{
    AFortPlayerState* FortState = Cast<AFortPlayerState>(Candidate);
    if (FortState && FortState->NativeMatchesSpectatedUniqueId216(Id))
    {
        return FortState;
    }
}
return nullptr;
