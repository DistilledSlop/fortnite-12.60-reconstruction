// Behaviour snippet: AFortPlayerPawn::IsSkydivingFromBus (body only; spliced into the generated member function).
// Original address: 0x142a7d480
// Decompile source: porting/generated/decomp/AFortPlayerPawn/IsSkydivingFromBus.c
// Proxy score: 92.2 (portability.json)
// Inferred: Decompile tests (byte +0x10e7 & 0x5) == 0x5: bits 0x4 (bIsSkydivingFromBus) and 0x1 (bLocalIsSkydiving) both set (or8 setters).
// Declared in generated header: no (the generator emits this declaration when the behaviour splice is on)
// CONFIDENCE: MEDIUM
return bIsSkydivingFromBus && bLocalIsSkydiving;
