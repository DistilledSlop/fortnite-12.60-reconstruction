// Original native body: 0x141a4b0d0.
// CONFIDENCE: HIGH
const UFortPlaylistAthena* PlaylistData = Playlist;
if (PlaylistData == nullptr)
{
    return false;
}
const FGameplayTag& Tag = FFortNativeGameplayTagsRuntime::Get().AthenaPlayground();
return PlaylistData->GameplayTagContainer.HasTag(Tag);
