// CONFIDENCE: high; native 0x141a74a60 iterates world player controllers and deactivates the first marker component.
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
                    Marker->Deactivate();
                }
            }
        }
    }
}
