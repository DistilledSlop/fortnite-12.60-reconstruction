// Original native body: 0x141a4b0d0.
// CONFIDENCE: HIGH
if (Playlist == nullptr)
{
    return false;
}
const FGameplayTag& Tag = FFortNativeGameplayTagsRuntime::Get().AthenaPlayground();
return Playlist->GameplayTagContainer.HasTag(Tag);
