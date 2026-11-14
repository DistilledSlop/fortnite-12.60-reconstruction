    // CONFIDENCE: HIGH
    // 0x141f34f00: equality compares both FName comparison index and number.
    for (UFortPlaylistAthena* Playlist : AthenaPlaylists)
    {
        if (Playlist && Playlist->PlaylistName == PlaylistName)
        {
            return Playlist;
        }
    }
    if (!PlaylistName.IsNone())
    {
        UE_LOG(LogFortPlaylistSelection, Error, TEXT("UFortPlaylistManager::GetPlaylist: Unable to find playlist %s"), *PlaylistName.ToString());
    }
    return nullptr;
