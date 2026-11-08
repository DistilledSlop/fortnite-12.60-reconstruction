// Original body address: 0x141ab1710
// Offset evidence: [RCX+0x260] = AFortAthenaMutator::CachedGameState (reflected). Null game state or null playlist (GetCurrentPlaylistAthena) returns false; otherwise CMP byte ptr [playlist+0x144],1 / SETNZ returns DBNOType != 1 (UFortPlaylist::DBNOType, reflected EDBNOType : uint8).
// CONFIDENCE: HIGH
static_assert(sizeof(AFortGameStateAthena) > 0, "AFortGameStateAthena must be complete for GetCurrentPlaylistAthena");
static_assert(sizeof(UFortPlaylistAthena) > 0, "UFortPlaylistAthena must be complete for DBNOType access");
AFortGameStateAthena* PlaylistGameState = CachedGameState;
if (PlaylistGameState == nullptr)
{
    return false;
}
UFortPlaylistAthena* CurrentPlaylist = PlaylistGameState->GetCurrentPlaylistAthena();
if (CurrentPlaylist == nullptr)
{
    return false;
}
return static_cast<uint8>(CurrentPlaylist->DBNOType) != 1;
