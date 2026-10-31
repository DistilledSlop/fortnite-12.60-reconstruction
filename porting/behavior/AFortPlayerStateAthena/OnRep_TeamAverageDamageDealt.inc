// Behaviour snippet: AFortPlayerStateAthena::OnRep_TeamAverageDamageDealt (body only; spliced into the generated member function).
// Original address: 0x142a8dda0
// Decompile source: porting/generated/decomp_improved/AFortPlayerStateAthena/OnRep_TeamAverageDamageDealt.c
// Proxy score: 86.9 (portability.json)
// Inferred: Decompile calls the multicast dispatcher 0x1406a9dc0 (TMulticastScriptDelegate::ProcessMulticastDelegate, see porting/decompilation_notes.md, third pass) on the delegate member with a parameter block holding the value(s) below. The decompile converts TeamAverageDamageDealt (uint16 in the header) to float for the single float parameter.
// Declared in generated header: yes
// CONFIDENCE: HIGH
TeamAverageDamageChanged.Broadcast(static_cast<float>(TeamAverageDamageDealt));
