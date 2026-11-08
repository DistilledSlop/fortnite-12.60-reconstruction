// Helper 0x1420327f0 (AFortAthenaMutator_WarmupMovie). Asm: MOV RAX,[RCX+0x2a8]; RET (no null check).
// Callers pass objects of several classes, so the parameter is UObject* and the field is read raw.
// CONFIDENCE: MEDIUM
return *reinterpret_cast<UObject* const*>(reinterpret_cast<const uint8*>(Owner) + 0x2a8);
