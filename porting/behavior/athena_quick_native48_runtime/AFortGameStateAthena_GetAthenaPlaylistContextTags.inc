// Original native: 0x141991730; reflected exec: 0x14291e7c0.
// CONFIDENCE: HIGH
FGameplayTagContainer Result;
const UFortPlaylistAthena* Playlist = GetCurrentPlaylistAthena();
if (Playlist != nullptr)
{
    Result = Playlist->GameplayTagContainer;
    if (Playlist->bIgnoreDefaultQuests)
    {
        Result.RemoveTag(FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x1720), false);
        Result.RemoveTag(FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x1728), false);
        Result.RemoveTag(FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x1730), false);
        Result.RemoveTag(FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x1738), false);
        Result.RemoveTag(FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x1778), false);
    }
    else
    {
        uint32 TagOffset = 0;
        switch (Playlist->MaxTeamSize)
        {
        case 1: TagOffset = 0x1720; break;
        case 2: TagOffset = 0x1728; break;
        case 3: TagOffset = 0x1730; break;
        case 4: TagOffset = 0x1738; break;
        default:
            if (Playlist->MaxTeamSize > 4) TagOffset = 0x1778;
            break;
        }
        if (TagOffset != 0)
        {
            Result.AddTag(FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(TagOffset));
        }
    }
}
return Result;
