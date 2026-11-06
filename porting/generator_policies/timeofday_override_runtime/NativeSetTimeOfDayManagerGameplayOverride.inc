// CONFIDENCE: HIGH
AFortGameStateAthena* AthenaState = Cast<AFortGameStateAthena>(GameState);
if (AthenaState && AthenaState->IsCreativeOrPlaygroundModePlaylist())
{
    UE_LOG(LogFort, Warning, TEXT("AFortGameModeAthena::SetTimeOfDayManagerGameplayOverride is not allowed in playgrounds or creative until their use of time of day managers is refactored"));
    return;
}
UClass* PreviousClass = TimeOfDayManagerGameplayOverride.Get();
UClass* CheckedNewClass = TimeOfDayManagerClass && TimeOfDayManagerClass->IsChildOf(AFortTimeOfDayManager::StaticClass())
    ? TimeOfDayManagerClass : nullptr;
if (PreviousClass != CheckedNewClass)
{
    TimeOfDayManagerGameplayOverride = TimeOfDayManagerClass;
    AFortWorldSettings* Settings = Cast<AFortWorldSettings>(GetWorld()->GetWorldSettings(false, true));
    if (Settings)
    {
        Settings->NativeRefreshTimeOfDayManager();
    }
}
