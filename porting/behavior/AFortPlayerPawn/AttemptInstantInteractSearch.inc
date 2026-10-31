// Behaviour snippet: AFortPlayerPawn::AttemptInstantInteractSearch (body only; spliced into the generated member function).
// Original address: 0x142a7adf0
// Decompile source: porting/generated/decomp_improved/AFortPlayerPawn/AttemptInstantInteractSearch.c
// Proxy score: 86.5 (portability.json)
// Inferred: Decompile calls the multicast dispatcher 0x1406a9dc0 (TMulticastScriptDelegate::ProcessMulticastDelegate, see porting/decompilation_notes.md, third pass) on the delegate member with a parameter block holding the value(s) below. The parameter block is NULL.
// Declared in generated header: no (the generator emits this declaration when the behaviour splice is on)
// CONFIDENCE: HIGH
OnAttemptedInstantInteractSearch.Broadcast();
