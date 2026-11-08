// Helper body for AFortWindManager static helper at 0x141dbf6b0 (sret by-value return: RCX = hidden return buffer, RDX = source; returns RAX = RCX).
// Asm: copies 0x90 bytes field-by-field (qword +0x0, dwords +0x8/+0x10..+0x34, bytes +0xc/+0xd, 16-byte moves +0x38/+0x54/+0x70,
// doubles +0x48/+0x64/+0x80, dwords +0x50/+0x6c/+0x88/+0x8c). No branches, no calls.
// Callers: AFortWindManager::GetCurrentWindImpulseCylinderDelta 0x141dcb550 (RDX = [this+0x2b0]), SetWindImpulseCylinderMagnitude 0x141dda630.
// Struct size 0x90 is not checked against reflection; a memberwise copy of FFortWindImpulseCylinder is assumed to cover the same bytes.
// CONFIDENCE: MEDIUM
return SourceCylinder;
