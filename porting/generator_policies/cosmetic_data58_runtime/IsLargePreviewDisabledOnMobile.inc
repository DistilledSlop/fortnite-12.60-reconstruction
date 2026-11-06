// Original native body: 0x141a4ad90.
// CONFIDENCE: HIGH
const UFortPlaylistAthena* PlaylistData = Playlist;
if (PlaylistData == nullptr)
{
    return false;
}
const FGameplayTag& Tag = FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x2538);
return PlaylistData->GameplayTagContainer.HasTag(Tag);
