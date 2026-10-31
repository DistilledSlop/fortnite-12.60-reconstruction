// Behaviour snippet: AFortPlayerPawn::HasControlledRCPawn (body only; spliced into the generated member function).
// Original address: 0x142a7cc70
// Decompile source: porting/generated/decomp/AFortPlayerPawn/HasControlledRCPawn.c
// Proxy score: 76.2 (portability.json)
// Inferred: Decompile calls FWeakObjectPtr_IsValid (0x142ee7880, default bEvenIfPendingKill = false) on the weak pointer at +0x26a0; reflection name ControlledRCPawn (TWeakObjectPtr in AFortPlayerPawn.h).
// Declared in generated header: no (the generator emits this declaration when the behaviour splice is on)
// CONFIDENCE: HIGH
return ControlledRCPawn.IsValid();
