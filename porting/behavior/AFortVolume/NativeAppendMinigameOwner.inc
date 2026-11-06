// CONFIDENCE: HIGH. 0x141c5f860, with 0x141c74820 inlined.
if (!AFortVolumeManager::NativeResolveVolumeManager(Volume))
{
    return;
}

UFortMinigameVolumeComponent* Component = static_cast<UFortMinigameVolumeComponent*>(
    Volume->FindComponentByClass(UFortMinigameVolumeComponent::StaticClass()));
AFortMinigame* Minigame = Component ? Component->CurrentMinigame : nullptr;
if (Minigame)
{
    const uint8 State = static_cast<uint8>(Minigame->CurrentState);
    if (State >= 1 && State <= 5)
    {
        OutOwners.AddUnique(Minigame);
    }
}
