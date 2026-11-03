// Original native body: 0x141a4a6f0.
// CONFIDENCE: HIGH
if (Playlist == nullptr)
{
    return false;
}
const FGameplayTag& Tag = FFortNativeGameplayTagsRuntime::Get().AthenaCreative();
return Playlist->GameplayTagContainer.HasTag(Tag) && Playlist->bEnableCreativeMode;
