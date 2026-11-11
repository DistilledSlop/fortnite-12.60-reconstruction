// CONFIDENCE: MEDIUM. Returns the settings actor of the creative volume that contains ContextActor. A context that is already a volume is used directly. Otherwise the volume comes from FindVolumeForActor on the volume manager for the actor. Returns null when no volume or minigame volume component is found.
// Original addresses: exec 0x142b3bad0 (thunk; body 0x141c7c460 is a direct call). Helpers: 0x142b32e10 (AFortVolume class), 0x141c74820 (GetVolumeManagerForActor), 0x141c7c5c0 (FindVolumeForActor), 0x1429da540 (UFortMinigameVolumeComponent class), virtual slot 0x610 (FindComponentByClass).

AFortVolume* Volume = nullptr;
if (ContextActor != nullptr && ContextActor->IsA(AFortVolume::StaticClass()))
{
    Volume = Cast<AFortVolume>(ContextActor);
}
else
{
    AFortVolumeManager* VolumeManager = GetVolumeManagerForActor(ContextActor);
    if (VolumeManager == nullptr)
    {
        return nullptr;
    }
    Volume = FindVolumeForActor(VolumeManager, ContextActor);
}

if (Volume == nullptr)
{
    return nullptr;
}

UFortMinigameVolumeComponent* MinigameComponent = Cast<UFortMinigameVolumeComponent>(Volume->FindComponentByClass(UFortMinigameVolumeComponent::StaticClass()));
if (MinigameComponent == nullptr)
{
    return nullptr;
}

return MinigameComponent->SettingsActor;
