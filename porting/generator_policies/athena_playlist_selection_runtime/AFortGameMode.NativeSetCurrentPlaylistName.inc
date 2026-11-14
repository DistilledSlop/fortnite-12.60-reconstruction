    // CONFIDENCE: HIGH
    // 0x141e7ead0 (+0x920). Manager must have been initialized by its owner.
    UFortPlaylistManager* Manager = GetPlaylistManagerNative();
    UFortPlaylistAthena* Playlist = Manager->GetPlaylistByNameNative(PlaylistName);
    if (Playlist)
    {
        CurrentPlaylistName = Playlist->PlaylistName;
        CurrentPlaylistId = Playlist->PlaylistId;
        UE_LOG(LogFortPlaylistSelection, Log, TEXT("AFortGameMode::SetCurrentPlaylistName(%s) set playlist to (%s, ID: %d)"), *PlaylistName.ToString(), *Playlist->PlaylistName.ToString(), Playlist->PlaylistId);
    }
    else
    {
        UE_LOG(LogFortPlaylistSelection, Error, TEXT("AFortGameMode::SetCurrentPlaylistName(%s) failed to find specified playlist"), *PlaylistName.ToString());
    }
