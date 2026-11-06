// BLOCKED SOURCE CANDIDATE, not included by the build.
// Required declaration on AFortPawn: virtual bool NativeVolumeClientPredicate() const;
// Required override on AFortPlayerPawn: bool NativeVolumeClientPredicate() const override;
// NativeVolumeClientPredicate bodies in image: AFortPawn false (0x1406801e0),
// AFortPlayerPawn bIsLocalPlayer (0x14194f570). Other subclass overrides require audit.
// NativeVolumeExcludedComponentTag() is intentionally unresolved: see HANDOFF.md.

void AFortVolume::NativeUpdateClientPostProcess(AActor* Actor, bool bEntering)
{
    AFortPawn* Pawn = Cast<AFortPawn>(Actor);
    if (!Pawn) return;
    UWorld* World = GetWorld();
    if (World)
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
        if ((!FirstController || FirstController != ViewingController) && !Pawn->NativeVolumeClientPredicate())
            return;
    }
    for (UActorComponent* Component : GetComponents())
    {
        UPostProcessComponent* PostProcess = Cast<UPostProcessComponent>(Component);
        if (PostProcess && !PostProcess->ComponentHasTag(NativeVolumeExcludedComponentTag()))
            PostProcess->BlendWeight = bEntering ? 1.0f : 0.0f;
    }
}
