// Behaviour snippet: AFortGameStateAthena::GetActivePlaylist (kind helper: a non-reflected member; body only).
// Original address: 0x141991bf0
// Decompile source: porting/generated/callee_probe/blocking/0x141991bf0.c (disassembly in the .asm beside it)
// Proxy score: none (queue row 5 of porting/behavior/CALLEE_PORT_QUEUE.md; not a portability record)
// Inferred: the function reads the qword at this+0x1df8 and returns it when it is non-zero, else the qword at
//   this+0x1df0. It makes no call and touches nothing else. The pair is CurrentPlaylistInfo.OverridePlaylist and
//   CurrentPlaylistInfo.BasePlaylist: the struct member CurrentPlaylistInfo is at offset 0x1cd0 (reflection offset
//   7376), and inside FPlaylistPropertyArray OverridePlaylist is at 296 (0x128) and BasePlaylist at 288 (0x120).
// Declared in generated header: no (helper: the generator declares it, see porting/behavior/README.md, "helper")
// CONFIDENCE: HIGH
if (CurrentPlaylistInfo.OverridePlaylist != nullptr)
{
	return CurrentPlaylistInfo.OverridePlaylist;
}
return CurrentPlaylistInfo.BasePlaylist;
