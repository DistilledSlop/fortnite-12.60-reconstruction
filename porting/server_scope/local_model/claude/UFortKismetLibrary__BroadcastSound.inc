// CONFIDENCE: MEDIUM. Disabled in Athena: when the world's game state is AFortGameStateAthena the call logs a warning and returns. Otherwise every AFortPlayerController in the world plays InSound through its sound function, with the given volume and pitch.
// Original address: 0x142967640 (exec). Native body: 0x141ee7f70. Helpers: 0x144640410 (world from object), 0x14468fb70 (world player controller list at +0x1c8), 0x142a3d9f0 (AFortPlayerController class), 0x142a3d8d0 (UFunction call on a controller).
UWorld* World = GetWorldFromContextObject(WorldContextObject);
if (World == nullptr)
{
    return;
}
if (World->GetGameState() != nullptr && World->GetGameState()->IsA(AFortGameStateAthena::StaticClass()))
{
    UE_LOG(LogFortKismet, Warning, TEXT("UFortKismetLibrary::BroadcastSound is no longer allowed in Athena"));
    return;
}
for (const TWeakObjectPtr<APlayerController>& WeakPC : World->PlayerControllerList)
{
    AFortPlayerController* PC = Cast<AFortPlayerController>(WeakPC.Get());
    if (PC != nullptr)
    {
        PC->PlaySoundOnController(InSound, VolumeMultiplier, PitchMultiplier);
    }
}
