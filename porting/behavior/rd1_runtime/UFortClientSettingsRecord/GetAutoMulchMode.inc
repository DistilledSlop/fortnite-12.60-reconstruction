// Original body address: 0x141e26370 (thunk 0x14287d660). Body asm: 40 instructions.
// Offset evidence: AutoMulchModeMap is a reflected TMap at +0x7d0 (hash probe at +0x7d8..+0x818 is its internal layout). Element key byte at +0, value byte at +1, stride 12. Missing key returns 0.
// CONFIDENCE: MEDIUM
const EFortAutoMulchMode* Found = AutoMulchModeMap.Find(Category);
if (Found == nullptr) { return static_cast<EFortAutoMulchMode>(0); }
return *Found;
