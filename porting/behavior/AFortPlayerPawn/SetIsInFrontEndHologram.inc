// Behaviour snippet: AFortPlayerPawn::SetIsInFrontEndHologram (body only; spliced into the generated member function).
// Original address: 0x142a803c0
// Decompile source: porting/generated/decomp/AFortPlayerPawn/SetIsInFrontEndHologram.c
// Proxy score: 89.8 (portability.json)
// Inferred: Decompile clears bit 0x40 and sets it from the argument in byte +0x10ea. bool_bits.py maps (0x10ea, 0x40) to bIsInFrontEndHologram (or8 setter).
// Declared in generated header: no (the generator emits this declaration when the behaviour splice is on)
// CONFIDENCE: HIGH
bIsInFrontEndHologram = bInFrontEndHologram;
