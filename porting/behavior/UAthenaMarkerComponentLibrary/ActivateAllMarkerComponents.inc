// CONFIDENCE: high; native 0x141a6d8f0 iterates world player controllers and activates the first marker component.
{
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (World)
    {
        for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
        {
            if (AFortPlayerController* Player = Cast<AFortPlayerController>(It->Get()))
            {
                if (UActorComponent* Marker = Player->FindComponentByClass(UAthenaMarkerComponent::StaticClass()))
                {
                    Marker->Activate(false);
                }
            }
        }
    }
}
