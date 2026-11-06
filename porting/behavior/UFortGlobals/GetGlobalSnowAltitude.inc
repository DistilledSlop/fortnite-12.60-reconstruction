// Behaviour snippet: UFortGlobals::GetGlobalSnowAltitude (body only; spliced into the generated member function).
// Original address: 0x142933e50
// Decompile source: porting/generated/native210-fresh-bulk/0x142933e50.c
// Inferred: calls the checked singleton accessor 0x141ed2350 (UFortGlobals::NativeGetGlobals) and returns the float
// GlobalSnowAltitude at +0x2cc (reflection offset 716, UFortGlobals).
// Declared in generated header: no (the generator emits this declaration when the behaviour splice is on)
// CONFIDENCE: HIGH
return UFortGlobals::NativeGetGlobals()->GlobalSnowAltitude;
