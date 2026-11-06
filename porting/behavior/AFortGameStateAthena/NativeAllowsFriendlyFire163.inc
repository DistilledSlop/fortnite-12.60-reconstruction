// CONFIDENCE: high
// Original address: 0x141989720
// 0x141989720. Tag override precedes volume/minigame and playlist rules.
const FGameplayTag& Override = FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0xb80);
if (Tags.HasTag(Override))
{
    return true;
}
AFortVolume* Volume = VolumeManager ? VolumeManager->NativeFindActorVolume(Pawn) : nullptr;
if (VolumeManager && Volume)
{
    AFortMinigame* Minigame = UFortMinigameVolumeComponent::NativeResolveMinigameFromVolume(Volume);
    if (Minigame)
    {
        const uint8 State = static_cast<uint8>(Minigame->CurrentState);
        if (State >= 1 && State <= 5)
        {
            return Minigame->NativeAllowsFriendlyFire163(Pawn);
        }
    }
}
return FriendlyFireType == EFriendlyFireType::On;
