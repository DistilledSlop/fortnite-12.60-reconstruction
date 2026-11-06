// CONFIDENCE: high; 0x141c794e0 resolves world, Athena game state, its volume manager and the player's volume, then finds its minigame component.
if (!PlayerState)
{
    return nullptr;
}
UWorld* World = PlayerState->GetWorld();
AFortGameStateAthena* GameState = World ? Cast<AFortGameStateAthena>(World->GetGameState()) : nullptr;
AFortVolumeManager* Manager = GameState ? GameState->NativeGetVolumeManager() : nullptr;
AFortVolume* Volume = Manager ? Manager->NativeFindPlayerVolume(PlayerState) : nullptr;
return UFortMinigameVolumeComponent::NativeResolveMinigameFromVolume(Volume);
