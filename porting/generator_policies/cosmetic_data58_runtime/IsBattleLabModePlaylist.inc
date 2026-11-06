// Original native body: 0x141a4a300.
// CONFIDENCE: HIGH
const UFortPlaylistAthena* PlaylistData = Playlist;
if (PlaylistData == nullptr)
{
    return false;
}
const FGameplayTag& Tag = FFortNativeGameplayTagsRuntime::Get().AthenaBattleLab();
return PlaylistData->GameplayTagContainer.HasTag(Tag);
