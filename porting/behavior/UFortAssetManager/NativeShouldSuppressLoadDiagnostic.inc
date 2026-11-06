// CONFIDENCE: HIGH
// Original address: 0x141e04570.
if (!GEngine || !GEngine->IsInitialized())
{
    return true;
}
bool bSuppress = true;
for (const FWorldContext& Context : GEngine->GetWorldContexts())
{
    UWorld* World = Context.World();
    if (!World || !World->IsGameWorld() || World->GetNetMode() == NM_DedicatedServer ||
        Cast<AFortGameModeFrontEnd>(World->GetAuthGameMode()))
    {
        continue;
    }
    if (NativeGetFortStartupPhase() <= 3 ||
        (World->DemoNetDriver && World->DemoNetDriver->IsLoadingCheckpoint()))
    {
        continue;
    }
    UFortGameInstance* GameInstance = Cast<UFortGameInstance>(Context.OwningGameInstance);
    if (GameInstance)
    {
        // Release the temporary weak reference before inspecting the pinned object.
        TSharedPtr<FFortPlayerLoadingScreenState> Controller =
            TWeakPtr<FFortPlayerLoadingScreenState>(GameInstance->NativePlayerLoadingScreen).Pin();
        if (Controller.IsValid())
        {
            bSuppress = bSuppress && Controller->bIsShown;
        }
    }
}
return bSuppress;
