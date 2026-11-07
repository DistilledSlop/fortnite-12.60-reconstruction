// Original body 0x141ec8790 (thunk 0x14286d2e0). Offset evidence: no this or field access. The only calls are FMemory::Free (0x142bee710) on the Data pointers of by-value FString parameters, each when non-null; those are the destructors the C++ parameters already run, so nothing is ported.
// CONFIDENCE: HIGH
return;
