// Original native body: 0x141a4a530.
// CONFIDENCE: HIGH
const UFortPlaylistAthena* PlaylistData = Playlist;
if (PlaylistData == nullptr)
{
    return false;
}
const FGameplayTag& Tag = FFortNativeGameplayTagsRuntime::Get().AthenaCreativeLTM();
return PlaylistData->GameplayTagContainer.HasTag(Tag);
