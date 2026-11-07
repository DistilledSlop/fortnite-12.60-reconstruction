// Original body 0x1419d7be0 (thunk 0x142a08d30). Offset evidence: same FVector members as GetSeatSteerDeltas (+0x12d8, +0x12e4, +0x12fc).
// Returns (a*f - d*c)*h + (c*e - b*f)*g + (d*b - a*e)*i.
// CONFIDENCE: HIGH
const float SA = NativeRecoveredFVector_0x12d8.X;
const float SB = NativeRecoveredFVector_0x12d8.Y;
const float SC = NativeRecoveredFVector_0x12d8.Z;
const float SD = NativeRecoveredFVector_0x12e4.X;
const float SE = NativeRecoveredFVector_0x12e4.Y;
const float SF = NativeRecoveredFVector_0x12e4.Z;
const float SG = NativeRecoveredFVector_0x12fc.X;
const float SH = NativeRecoveredFVector_0x12fc.Y;
const float SI = NativeRecoveredFVector_0x12fc.Z;
const float SQ = NativeRecoveredFVector_0x126c.X;
const float SR = NativeRecoveredFVector_0x126c.Y;
const float SP = NativeRecoveredFVector_0x126c.Z;
return (SA * SF - SD * SC) * SH + (SC * SE - SB * SF) * SG + (SD * SB - SA * SE) * SI;
