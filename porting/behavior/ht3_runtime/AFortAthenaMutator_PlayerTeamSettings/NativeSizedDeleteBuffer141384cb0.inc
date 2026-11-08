// Original body address: 0x141384cb0 (helper; tail-jumps to FMemory::Free at 0x142bee710)
// Asm: JMP 0x142bee710 with RCX unchanged, so the first argument is the pointer to release. EDX (the class size callers pass, e.g. 0x2d8 and 0xb98) is ignored by the asm.
// Callers such as UCommonActivatablePanel::OnBeginOutro (0x1407a7910) and ABuildingProp_LockDevice::LockObject (0x141bfb1d0) call it on the flag-gated delete path with this and the class size.
// The free is the entire behavior of this helper, so it is written out here rather than omitted.
// CONFIDENCE: HIGH
FMemory::Free(Pointer);
(void)AllocationSize;
