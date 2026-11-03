// Behaviour snippet: AFortGameStateAthena::GetCurrentPlaylistName (body only).
// Original address: 0x14291e8b0; native body 0x141991ce0.
// Decompile source: porting/generated/decomp/AFortGameStateAthena/GetCurrentPlaylistName.c
// Evidence: porting/behavior/game_state_queries_evidence.md
// CONFIDENCE: HIGH
UFortPlaylistAthena* Playlist = GetCurrentPlaylistAthena();
return Playlist != nullptr ? Playlist->PlaylistName : FName();
