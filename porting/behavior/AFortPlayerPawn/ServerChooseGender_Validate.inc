// Behaviour snippet: AFortPlayerPawn::ServerChooseGender_Validate (body only; spliced into the generated member function).
// Original address: 0x14234a140
// Decompile source: porting/generated/decomp_improved/AFortPlayerPawn/ServerChooseGender_Validate.c
// Proxy score: 73.2 (portability.json)
// Inferred: Decompile returns the unsigned argument < 4 (the one argument after this; the exec passes Gender only). Unique body at 0x14234a140. The name comes from the RPC vtable slot naming, so MEDIUM.
// Declared in generated header: no (the generator emits `bool <Name>_Validate(params)` with the WithValidation specifier when the behaviour splice is on)
// CONFIDENCE: MEDIUM
return Gender < 4;
