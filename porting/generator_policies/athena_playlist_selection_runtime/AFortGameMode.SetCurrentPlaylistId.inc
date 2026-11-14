    // CONFIDENCE: HIGH
    // 0x141e7e9e0: unsigned comparison rejects negative IDs as well as IDs >= 2.
    if (static_cast<uint32>(PlaylistId) < 2u)
    {
        UE_LOG(LogFortPlaylistSelection, Log, TEXT("AFortGameModeAthena::SetCurrentPlaylistId(%d) set playlist to (NAME_None, ID: %d)"), PlaylistId, PlaylistId);
        CurrentPlaylistId = PlaylistId;
        CurrentPlaylistName = NAME_None;
    }
    else
    {
        UE_LOG(LogFortPlaylistSelection, Error, TEXT("AFortGameMode::SetCurrentPlaylistId(%d) failed to find specified playlist"), PlaylistId);
    }
