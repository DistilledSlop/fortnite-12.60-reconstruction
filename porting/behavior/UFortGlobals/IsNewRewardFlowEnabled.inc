// Behaviour snippet: UFortGlobals::IsNewRewardFlowEnabled (body only; spliced into the generated member function).
// Original address: 0x1429349e0
// Decompile source: porting/generated/native210-fresh-bulk/0x1429349e0.c
// Inferred: calls the checked singleton accessor 0x141ed2350 (UFortGlobals::NativeGetGlobals) and returns the byte at
// +0xc3 (reflection bool bEnableNewRewardFlow, offset 195, mask 1, mov8).
// Declared in generated header: no (the generator emits this declaration when the behaviour splice is on)
// CONFIDENCE: HIGH
return UFortGlobals::NativeGetGlobals()->bEnableNewRewardFlow;
