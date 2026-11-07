// Original address: 0x1419b0720
// Offset evidence: [this+0x1df8] then [this+0x1df0] are unreflected pointers. No reflected member covers 0x1df0 or 0x1df8 in AFortGameStateAthena or its generated ancestors (highest reflected member below them is CurrentPlaylistInfo at 0x1cd0). Recovered as NativeRecoveredPtr_0x1df8 and NativeRecoveredPtr_0x1df0.
// Callee 0x141a5ba60 -> UFortPlaylistAthenaHelper::ShouldEnterSpawningStartup(UFortPlaylistAthena* Playlist) (static, declared in UFortPlaylistAthenaHelper.h).
// Asm: MOV RCX,[this+0x1df8]; TEST RCX,RCX; JNZ -> JMP 0x141a5ba60; else MOV RCX,[this+0x1df0]; JMP 0x141a5ba60 (tail call, bool result returned in AL).
// CONFIDENCE: MEDIUM
UFortPlaylistAthena* Playlist = reinterpret_cast<UFortPlaylistAthena*>(NativeRecoveredPtr_0x1df8);
if (Playlist == nullptr)
{
	Playlist = reinterpret_cast<UFortPlaylistAthena*>(NativeRecoveredPtr_0x1df0);
}
return UFortPlaylistAthenaHelper::ShouldEnterSpawningStartup(Playlist);
