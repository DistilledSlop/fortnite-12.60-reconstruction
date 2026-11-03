// CONFIDENCE: high; complete native 0x141c795a0, preserving actor virtual dispatch.
if (!Volume)
{
    return nullptr;
}
UFortMinigameVolumeComponent* Component = static_cast<UFortMinigameVolumeComponent*>(
    Volume->FindComponentByClass(UFortMinigameVolumeComponent::StaticClass()));
return Component ? Component->CurrentMinigame : nullptr;
