// CONFIDENCE: HIGH
AFortPawn* Pawn = Cast<AFortPawn>(Actor);
if (!Pawn)
{
    return;
}
if (UWorld* World = GetWorld())
{
    APlayerController* ViewingController = nullptr;
    if (UWorld* PawnWorld = Pawn->GetWorld())
    {
        for (FConstPlayerControllerIterator It = PawnWorld->GetPlayerControllerIterator(); It; ++It)
        {
            APlayerController* Controller = It->Get();
            if (Controller && Controller->IsLocalController() && Controller->GetViewTarget() == Pawn)
            {
                ViewingController = Controller;
                break;
            }
        }
    }
    APlayerController* FirstController = World->GetFirstPlayerController();
    if ((!FirstController || FirstController != ViewingController) && !Pawn->NativeIsLocalPlayer())
    {
        return;
    }
}
for (UActorComponent* Component : GetComponents())
{
    UPostProcessComponent* PostProcess = Cast<UPostProcessComponent>(Component);
    if (PostProcess && !PostProcess->ComponentHasTag(GFortVolumeMobilePostProcessTag))
    {
        PostProcess->BlendWeight = bEntering ? 1.0f : 0.0f;
    }
}
