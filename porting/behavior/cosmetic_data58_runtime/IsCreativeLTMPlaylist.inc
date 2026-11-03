// Original native body: 0x141a4a530.
// CONFIDENCE: HIGH
if (Playlist == nullptr)
{
    return false;
}
const FGameplayTag& Tag = FFortNativeGameplayTagsRuntime::Get().AthenaCreativeLTM();
return Playlist->GameplayTagContainer.HasTag(Tag);
