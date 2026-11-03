// Original native body: 0x141a49590.
// CONFIDENCE: HIGH
const UFortPlaylistAthena* PlaylistData = Playlist;
if (PlaylistData == nullptr)
{
    return false;
}
const FGameplayTag& Tag = FFortNativeGameplayTagsRuntime::Get().AthenaCreative();
return PlaylistData->GameplayTagContainer.HasTag(Tag);
