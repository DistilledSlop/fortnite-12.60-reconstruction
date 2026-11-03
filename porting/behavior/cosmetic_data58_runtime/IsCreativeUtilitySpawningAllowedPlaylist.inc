// Original native body: 0x141a4a7e0.
// CONFIDENCE: HIGH
if (Playlist == nullptr)
{
    return false;
}
const FGameplayTag& Tag = FFortNativeGameplayTagsRuntime::Get().AthenaCreativeUtilitySpawning();
return Playlist->GameplayTagContainer.HasTag(Tag);
