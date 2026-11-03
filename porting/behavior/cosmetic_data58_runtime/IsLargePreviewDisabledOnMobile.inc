// Original native body: 0x141a4ad90.
// CONFIDENCE: HIGH
if (Playlist == nullptr)
{
    return false;
}
const FGameplayTag& Tag = FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x2538);
return Playlist->GameplayTagContainer.HasTag(Tag);
