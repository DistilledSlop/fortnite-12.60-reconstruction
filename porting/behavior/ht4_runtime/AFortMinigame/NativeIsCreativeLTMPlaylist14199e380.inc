// Native helper 0x14199e380 (static helper on AFortMinigame; callers ABuildingActor, UFortKismetLibrary, AFortMinigame, AFortPlayerControllerAthena).
// Asm: RCX = [RAX+0x1df8]; if that is null, RCX = [RAX+0x1df0]; then JMP 0x141a4a530, the ported AFortGameStateAthena::PlaylistIsCreativeLTM,
// whose bool result is returned unchanged.
// Offsets 0x1df8 and 0x1df0 have no reflected members; raw pointer loads are used, typed by the callee's Playlist parameter.
// CONFIDENCE: MEDIUM
uint8* const ObjectBytes = reinterpret_cast<uint8*>(MinigameObject);
const UFortPlaylistAthena* Playlist = *reinterpret_cast<const UFortPlaylistAthena* const*>(ObjectBytes + 0x1df8);
if (Playlist == nullptr)
{
	Playlist = *reinterpret_cast<const UFortPlaylistAthena* const*>(ObjectBytes + 0x1df0);
}
return AFortGameStateAthena::PlaylistIsCreativeLTM(Playlist);
