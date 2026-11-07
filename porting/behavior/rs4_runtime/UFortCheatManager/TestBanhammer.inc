// Original body 0x142360ed0 (thunk 0x14286eee0). Offset evidence: no this or field access. The only calls are FMemory::Free (0x142bee710) on the Data pointers of by-value FString parameters BanReason (R9), BanhammerCustomMessage ([RSP+0x70]) and BanhammerDetails ([RSP+0x78]), each when non-null; those are the destructors the C++ parameters already run.
// CONFIDENCE: HIGH
return;
