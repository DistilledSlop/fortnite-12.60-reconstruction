// CONFIDENCE: high; native selects the first local controller from the viewport game instance and invokes the identified server event.
{
    if (GEngine->GameViewport)
    {
        if (UFortGameInstance* GameInstance = Cast<UFortGameInstance>(GEngine->GameViewport->GetGameInstance()))
        {
            if (AFortPlayerController* Controller = Cast<AFortPlayerController>(GameInstance->GetFirstLocalPlayerController()))
            {
                Controller->Cheat_ClearForcedCosmeticItems();
                return true;
            }
        }
    }
    return false;
}
