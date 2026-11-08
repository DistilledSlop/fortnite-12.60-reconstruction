// Helper 0x14199e3c0 on AFortGameModeAthena. Callers include AFortAthenaSKVehicle::HasPermissionToAlterBuildings (0x1418ef500),
// AFortPlayerController::ServerClearItemList (0x1423aa7f0); both pass RCX=GameMode.
// Evidence: playlist = qword [GameMode+0x1df8]; when null, qword [GameMode+0x1df0]; then tail-jump to AFortGameStateAthena::PlaylistIsCreativeMode (0x141a4a6f0), which returns the bool.
// CONFIDENCE: HIGH
const uint8* PrimaryPlaylist = *reinterpret_cast<const uint8* const*>(reinterpret_cast<const uint8*>(GameMode) + 0x1df8);
const uint8* ActivePlaylist = PrimaryPlaylist;
if (ActivePlaylist == nullptr)
{
	ActivePlaylist = *reinterpret_cast<const uint8* const*>(reinterpret_cast<const uint8*>(GameMode) + 0x1df0);
}
return AFortGameStateAthena::PlaylistIsCreativeMode(reinterpret_cast<const UFortPlaylistAthena*>(ActivePlaylist));
