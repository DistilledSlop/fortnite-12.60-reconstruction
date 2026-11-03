// CONFIDENCE: HIGH
// Behaviour snippet: AFortGameStateAthena::GetMaxPlaylistPlayers (body only).
// Original address: 0x14291e920; native body 0x141995550.
// Decompile source: porting/generated/decomp/AFortGameStateAthena/GetMaxPlaylistPlayers.c
// Evidence: porting/behavior/aircraft_state_evidence.md
// The original obtains the world's game state, which need not be this object.
UWorld* World = GetWorld();
AFortGameStateAthena* State = World ? Cast<AFortGameStateAthena>(World->GetGameState()) : nullptr;
if (State != nullptr)
{
	UFortPlaylistAthena* Playlist = State->GetCurrentPlaylistAthena();
	if (Playlist != nullptr)
	{
		return Playlist->MaxPlayers;
	}
}
return -1;
