// Original native body: 0x141a4a610.
// CONFIDENCE: HIGH
const UFortPlaylistAthena* PlaylistData = Playlist;
if (PlaylistData == nullptr)
{
    return false;
}
const FGameplayTag& Tag = FFortNativeGameplayTagsRuntime::Get().AthenaCreativeMM();
return PlaylistData->GameplayTagContainer.HasTag(Tag);
