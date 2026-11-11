// CONFIDENCE: MEDIUM. Same as BroadcastSound, but the sound is played at Location. Disabled in Athena (logs a warning and returns). Every AFortPlayerController in the world receives InSound at Location with the given volume and pitch.
// Original address: 0x142967770 (exec). Native body: 0x141ee80a0. Helper: 0x142a3d920 (UFunction call on a controller with a location).
UWorld* World = GetWorldFromContextObject(WorldContextObject);
if (World == nullptr)
{
    return;
}
if (World->GetGameState() != nullptr && World->GetGameState()->IsA(AFortGameStateAthena::StaticClass()))
{
    UE_LOG(LogFortKismet, Warning, TEXT("UFortKismetLibrary::BroadcastSoundAtLocation is no longer allowed in Athena"));
    return;
}
for (const TWeakObjectPtr<APlayerController>& WeakPC : World->PlayerControllerList)
{
    AFortPlayerController* PC = Cast<AFortPlayerController>(WeakPC.Get());
    if (PC != nullptr)
    {
        PC->PlaySoundAtLocationOnController(InSound, Location, VolumeMultiplier, PitchMultiplier);
    }
}
