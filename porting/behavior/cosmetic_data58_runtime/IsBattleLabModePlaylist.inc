// Original native body: 0x141a4a300.
// CONFIDENCE: HIGH
if (Playlist == nullptr)
{
    return false;
}
const FGameplayTag& Tag = FFortNativeGameplayTagsRuntime::Get().AthenaBattleLab();
return Playlist->GameplayTagContainer.HasTag(Tag);
