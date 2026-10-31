// Behaviour snippet: AFortGameStateAthena::OnRep_WinningScore (body only; spliced into the generated member function).
// Original address: 0x142920860
// Decompile source: porting/generated/decomp_improved/AFortGameStateAthena/OnRep_WinningScore.c
// Proxy score: 86.9 (portability.json)
// Inferred: Decompile calls the multicast dispatcher 0x1406a9dc0 (TMulticastScriptDelegate::ProcessMulticastDelegate, see porting/decompilation_notes.md, third pass) on the delegate member with a parameter block holding the value(s) below. One int32 parameter: WinningScore (+0x142c).
// Declared in generated header: yes
// CONFIDENCE: HIGH
WinningScoreDetermined.Broadcast(WinningScore);
