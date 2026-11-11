// CONFIDENCE: HIGH. Returns the settings actor of a creative volume directly. Returns null when the volume is null or has no minigame volume component.
// Original addresses: exec 0x142b3bb50 (thunk; body 0x141c7c4e0 is a direct call). Helpers: 0x1429da540 (UFortMinigameVolumeComponent class), virtual slot 0x610 (FindComponentByClass).

if (ContextVolume == nullptr)
{
    return nullptr;
}

UFortMinigameVolumeComponent* MinigameComponent = Cast<UFortMinigameVolumeComponent>(ContextVolume->FindComponentByClass(UFortMinigameVolumeComponent::StaticClass()));
if (MinigameComponent == nullptr)
{
    return nullptr;
}

return MinigameComponent->SettingsActor;
