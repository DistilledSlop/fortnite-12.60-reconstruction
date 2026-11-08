// Helper 0x142c47190, host AFortDecoPreview (static, sret return). 9 instructions, RET-terminated, no callees.
// Asm: copies dwords [RDX+0], [RDX+4], [RDX+8] to the result at +0, +4, +8 and writes +0xc = 0x3f800000 (1.0f); returns the result pointer.
// Callers: ABuildingSMActor::StartBounceAnimation (0x141c1f1e0) pass RCX = address of a local FVector4 and RDX = source vector.
// CONFIDENCE: HIGH
FVector4 Result;
Result.X = InVector.X;
Result.Y = InVector.Y;
Result.Z = InVector.Z;
Result.W = 1.0f;
return Result;
