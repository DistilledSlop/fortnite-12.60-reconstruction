// Behaviour snippet: AFortPlayerStateAthena::OnRep_TeamScore (body only; spliced into the generated member function).
// Original address: 0x142a8de60
// Decompile source: porting/generated/decomp_improved/AFortPlayerStateAthena/OnRep_TeamScore.c
// Proxy score: 86.9 (portability.json)
// Inferred: Decompile calls the multicast dispatcher 0x1406a9dc0 (TMulticastScriptDelegate::ProcessMulticastDelegate, see porting/decompilation_notes.md, third pass) on the delegate member with a parameter block holding the value(s) below. One int32 parameter: TeamScore.
// Declared in generated header: yes
// CONFIDENCE: HIGH
TeamScoreChanged.Broadcast(TeamScore);
