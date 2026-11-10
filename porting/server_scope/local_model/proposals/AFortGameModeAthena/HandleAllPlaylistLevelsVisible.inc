// CONFIDENCE: HIGH, contingent on the native delegate and volume-manager dependencies in work/dependencies/playlist_visibility.json.
// Original exec/body: 0x1428f03e0; helpers 0x140690620, 0x141999ba0, 0x141c82200, 0x14199e3c0, 0x141c5fb10, 0x141c70f80.
UWorld* World = GetWorld();
AFortGameStateAthena* AthenaState = World ? Cast<AFortGameStateAthena>(World->GetGameState()) : nullptr;
if (AthenaState)
{
    AthenaState->OnAllAdditionalPlaylistLevelsVisibleNative.RemoveAll(this);
    AFortVolumeManager* VolumeManager = AthenaState->GetPlaylistVisibilityVolumeManager();
    if (VolumeManager && VolumeManager->IsInSpawningStartup() &&
        !AthenaState->IsCreativeModePlaylist() && !VolumeManager->HasUserIslandsLoading())
    {
        VolumeManager->EndSpawningStartup();
    }
}
