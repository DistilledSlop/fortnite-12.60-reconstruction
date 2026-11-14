    // CONFIDENCE: HIGH
    // 0x141963c70: deprecation is reported on every call, including success.
    UE_LOG(LogFortPlaylistSelection, Warning, TEXT("AFortGameModeAthena::SetCurrentPlaylistId() is deprecated; use SetCurrentPlaylistName() instead"));
    UFortPlaylistManager* Manager = GetPlaylistManagerNative();
    UFortPlaylistAthena* Playlist = Manager->GetPlaylistByIdNative(PlaylistId);
    if (Playlist)
    {
        CurrentPlaylistName = Playlist->PlaylistName;
        CurrentPlaylistId = Playlist->PlaylistId;
        UE_LOG(LogFortPlaylistSelection, Log, TEXT("AFortGameMode::SetCurrentPlaylistId(%d) set playlist to (%s, ID: %d)"), PlaylistId, *Playlist->PlaylistName.ToString(), Playlist->PlaylistId);
    }
    else
    {
        UE_LOG(LogFortPlaylistSelection, Error, TEXT("AFortGameModeAthena::SetCurrentPlaylistId(%d) failed to find specified playlist"), PlaylistId);
    }
