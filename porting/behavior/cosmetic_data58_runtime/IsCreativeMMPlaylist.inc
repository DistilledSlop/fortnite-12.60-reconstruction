// Original native body: 0x141a4a610.
// CONFIDENCE: HIGH
if (Playlist == nullptr)
{
    return false;
}
const FGameplayTag& Tag = FFortNativeGameplayTagsRuntime::Get().AthenaCreativeMM();
return Playlist->GameplayTagContainer.HasTag(Tag);
