// Behaviour snippet: AFortPlayerControllerAthena::OnRep_FocalPoint (body only; spliced into the generated member function).
// Original address: 0x142a59770
// Decompile source: porting/generated/decomp_improved/AFortPlayerControllerAthena/OnRep_FocalPoint.c
// Proxy score: 71.5 (portability.json)
// Inferred: Decompile calls the multicast dispatcher 0x1406a9dc0 (TMulticastScriptDelegate::ProcessMulticastDelegate, see porting/decompilation_notes.md, third pass) on the delegate member with a parameter block holding the value(s) below. One AActor* parameter: FocalPoint (+0x2b40).
// Declared in generated header: yes
// CONFIDENCE: HIGH
OnFocalPointActorChanged.Broadcast(FocalPoint);
