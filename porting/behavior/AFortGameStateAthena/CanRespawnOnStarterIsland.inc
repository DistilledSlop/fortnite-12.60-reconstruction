// Original address: 0x14291e1f0.
// CONFIDENCE: HIGH
// Evidence: porting/behavior/playlist_mode_evidence.md
UFortPlaylistAthena* InitialPlaylist = GetCurrentPlaylistAthena();
if (InitialPlaylist == nullptr)
{
	return false;
}
if (PlaylistIsCreativeMode(InitialPlaylist))
{
	return true;
}
return PlaylistIsPlayground(GetCurrentPlaylistAthena()) && InitialPlaylist->bUseCreativeStarterIsland;
