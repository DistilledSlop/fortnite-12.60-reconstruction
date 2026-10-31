// Behaviour snippet: AFortGameStateAthena::OnRep_StormCapState (body only; spliced into the generated member function).
// Original address: 0x142920710
// Decompile source: porting/generated/decomp_improved/AFortGameStateAthena/OnRep_StormCapState.c
// Proxy score: 71.5 (portability.json)
// Inferred: Decompile calls the multicast dispatcher 0x1406a9dc0 (TMulticastScriptDelegate::ProcessMulticastDelegate, see porting/decompilation_notes.md, third pass) on the delegate member with a parameter block holding the value(s) below. One byte parameter: StormCapState (uint8 at +0x1351).
// Declared in generated header: yes
// CONFIDENCE: HIGH
StormCapStateChanged.Broadcast(StormCapState);
