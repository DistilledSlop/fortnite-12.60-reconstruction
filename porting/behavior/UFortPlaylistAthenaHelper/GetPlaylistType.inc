// Original native body: 0x141a46dc0.
// CONFIDENCE: HIGH
const UFortPlaylistAthena* PlaylistData = Playlist;
if (PlaylistData == nullptr)
{
    return EFortPlaylistType::Default;
}
const FFortNativeGameplayTagsRuntime& Tags = FFortNativeGameplayTagsRuntime::Get();
if (PlaylistData->GameplayTagContainer.HasTag(Tags.AthenaBattleLab()))
{
    return EFortPlaylistType::BattleLab;
}
if (PlaylistData->GameplayTagContainer.HasTag(Tags.AthenaCreativeLTM()))
{
    return EFortPlaylistType::Creative_LTM;
}
if (PlaylistData->GameplayTagContainer.HasTag(Tags.AthenaCreative()) && PlaylistData->bEnableCreativeMode)
{
    return EFortPlaylistType::Creative;
}
if (PlaylistData->GameplayTagContainer.HasTag(Tags.AthenaPlayground()))
{
    return EFortPlaylistType::Playground;
}
return EFortPlaylistType::Default;
