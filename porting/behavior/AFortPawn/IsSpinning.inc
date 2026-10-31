// Behaviour snippet: AFortPawn::IsSpinning (body only; spliced into the generated member function).
// Original address: 0x142a1cd80
// Decompile source: porting/generated/decomp/AFortPawn/IsSpinning.c
// Proxy score: 92.9 (portability.json)
// Inferred: LocalSpin = float at +0x638 (reflection offset 1592, header AFortPawn.h). Decompile stores 0 when LocalSpin == 0.0 and 1 otherwise.
// Declared in generated header: no (generator must emit this declaration)
// CONFIDENCE: HIGH
return LocalSpin != 0.0f;
