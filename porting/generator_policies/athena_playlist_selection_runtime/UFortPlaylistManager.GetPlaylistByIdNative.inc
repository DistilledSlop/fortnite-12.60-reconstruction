    // CONFIDENCE: HIGH
    // 0x141f2b1f0: ordered first match; null entries are skipped.
    for (UFortPlaylistAthena* Playlist : AthenaPlaylists)
    {
        if (Playlist && Playlist->PlaylistId == PlaylistId)
        {
            return Playlist;
        }
    }
    return nullptr;
