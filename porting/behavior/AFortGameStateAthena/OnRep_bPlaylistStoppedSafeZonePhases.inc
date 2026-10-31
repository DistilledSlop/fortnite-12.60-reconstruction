// Behaviour snippet: AFortGameStateAthena::OnRep_bPlaylistStoppedSafeZonePhases (body only; spliced into the generated member function).
// Original address: 0x1429208c0
// Decompile source: porting/generated/decomp_improved/AFortGameStateAthena/OnRep_bPlaylistStoppedSafeZonePhases.c
// Proxy score: 71.5 (portability.json)
// Inferred: Decompile calls the multicast dispatcher 0x1406a9dc0 (TMulticastScriptDelegate::ProcessMulticastDelegate, see porting/decompilation_notes.md, third pass) on the delegate member with a parameter block holding the value(s) below. One bool parameter. The decompile reads byte +0x11f1 (named as TotalFinalCountdownTime[5] by Ghidra); bool_bits maps that byte to bPlaylistStoppedSafeZonePhases (mov8, mask 1).
// Declared in generated header: yes
// CONFIDENCE: HIGH
StormProgressStopped.Broadcast(bPlaylistStoppedSafeZonePhases);
