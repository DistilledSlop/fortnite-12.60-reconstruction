// Helper 0x1414da240 (10 callers; callers pass a block pointer in RCX and use RAX as a pointer, e.g. 0x144091870).
// Asm: MOV RAX,qword ptr [RCX]; RET. Reads the first qword of the source block.
// Offset evidence: the source block has no recorded owning class; raw read of 8 bytes at offset 0.
// CONFIDENCE: HIGH
return *reinterpret_cast<void* const*>(Source);
