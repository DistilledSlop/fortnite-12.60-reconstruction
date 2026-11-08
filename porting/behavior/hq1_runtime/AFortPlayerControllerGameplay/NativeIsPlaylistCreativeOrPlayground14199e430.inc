// Native helper 0x14199e430 (AFortPlayerControllerGameplay static). Returns true when the controller's playlist
// (raw pointer at +0x1df8, fallback +0x1df0) has the creative tag or is a playground playlist.
// Asm: RCX=[+0x1df8] or [+0x1df0] if null -> PlaylistHasCreativeTag(0x141a49590); true if set.
// Then RCX=[+0x1df8] or [+0x1df0] if null -> PlaylistIsPlayground(0x141a4b0d0); true if set. Else false.
// CONFIDENCE: HIGH
uint8* ControllerBytes = reinterpret_cast<uint8*>(ControllerObject);
UFortPlaylistAthena* PrimaryPlaylist = *reinterpret_cast<UFortPlaylistAthena**>(ControllerBytes + 0x1df8);
UFortPlaylistAthena* FallbackPlaylist = *reinterpret_cast<UFortPlaylistAthena**>(ControllerBytes + 0x1df0);
const UFortPlaylistAthena* CreativeCheckPlaylist = PrimaryPlaylist != nullptr ? PrimaryPlaylist : FallbackPlaylist;
if (AFortGameStateAthena::PlaylistHasCreativeTag(CreativeCheckPlaylist))
{
	return true;
}
PrimaryPlaylist = *reinterpret_cast<UFortPlaylistAthena**>(ControllerBytes + 0x1df8);
FallbackPlaylist = *reinterpret_cast<UFortPlaylistAthena**>(ControllerBytes + 0x1df0);
const UFortPlaylistAthena* PlaygroundCheckPlaylist = PrimaryPlaylist != nullptr ? PrimaryPlaylist : FallbackPlaylist;
if (AFortGameStateAthena::PlaylistIsPlayground(PlaygroundCheckPlaylist))
{
	return true;
}
return false;
