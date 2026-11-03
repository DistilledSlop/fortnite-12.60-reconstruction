// Original native body: 0x141a4a7e0.
// CONFIDENCE: HIGH
const UFortPlaylistAthena* PlaylistData = Playlist;
if (PlaylistData == nullptr)
{
    return false;
}
const FGameplayTag& Tag = FFortNativeGameplayTagsRuntime::Get().AthenaCreativeUtilitySpawning();
return PlaylistData->GameplayTagContainer.HasTag(Tag);
