// Behaviour snippet: AFortPlayerPawn::IsInSkyTube (body only; spliced into the generated member function).
// Original address: 0x142a7d230
// Decompile source: porting/generated/decomp/AFortPlayerPawn/IsInSkyTube.c
// Proxy score: 92.2 (portability.json)
// Inferred: CurrentSkyTube = AFortSkyTube* at +0x22d8 (reflection, AFortPlayerPawn.h). Decompile tests only for non-null.
// Declared in generated header: no (the generator emits this declaration when the behaviour splice is on)
// CONFIDENCE: HIGH
return CurrentSkyTube != nullptr;
