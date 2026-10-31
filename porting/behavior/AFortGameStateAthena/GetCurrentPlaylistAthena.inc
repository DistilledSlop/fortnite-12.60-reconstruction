// Behaviour snippet: AFortGameStateAthena::GetCurrentPlaylistAthena (kind helper: a non-reflected member; body only).
// Original address: 0x141991bf0 (Eighth pass name; the worked example of kinds helper and static_helper, earlier
//   named GetActivePlaylist, renamed in the ninth pass. The manifest has one entry for it, under the new name).
// Decompile source: porting/generated/callee_probe/blocking/0x141991bf0.c (disassembly in the .asm beside it)
// Proxy score: none (queue row 5 of porting/behavior/CALLEE_PORT_QUEUE.md; not a portability record)
// Inferred: the function reads the qword at this+0x1df8 and returns it when it is non-zero, else the qword at
//   this+0x1df0. It makes no call and touches nothing else. The pair is CurrentPlaylistInfo.OverridePlaylist and
//   CurrentPlaylistInfo.BasePlaylist: the struct member CurrentPlaylistInfo is at offset 0x1cd0 (reflection offset
//   7376), and inside FPlaylistPropertyArray OverridePlaylist is at 296 (0x128) and BasePlaylist at 288 (0x120).
// Access is public (ninth pass): AFortPlayerStateAthena and AFortGameModeAthena::SafeZoneInsideChecks call it.
// Declared in generated header: no (helper: the generator declares it, see porting/behavior/README.md, "helper")
// CONFIDENCE: HIGH
if (CurrentPlaylistInfo.OverridePlaylist != nullptr)
{
	return CurrentPlaylistInfo.OverridePlaylist;
}
return CurrentPlaylistInfo.BasePlaylist;
