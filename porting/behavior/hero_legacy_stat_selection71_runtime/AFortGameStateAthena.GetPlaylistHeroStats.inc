// Original address: 0x1419964f0.
// CONFIDENCE: HIGH
// 0x1419964f0. The current playlist accessor preserves override-before-base.
UFortPlaylistAthena* Playlist = GetCurrentPlaylistAthena();
if (Playlist == nullptr)
{
    return nullptr;
}
UDataTable* Table = Playlist->HeroStats.LoadSynchronous();
if (Table == nullptr)
{
    return nullptr;
}
return Table->FindRow<FFortPlayerPawnStats>(RowName, TEXT("AFortGameStateAthena::GetPlaylistHeroStats"), true);
