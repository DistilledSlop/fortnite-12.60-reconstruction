// Original body 0x1419d7ab0 (thunk 0x142a08bf0). ABI: RDX=&SeatSteerYawDelta, R8=&SeatSteerPitchDelta, R9=&SeatSteerRollDelta.
// Offset evidence: FVector at +0x12d8 (a,b,c) and +0x12e4 (d,e,f) are existing reflected-gap members NativeRecoveredFVector_0x12d8/0x12e4; FVector at +0x12fc (g,h,i), +0x126c (q=X, r=Y, p=Z) are new members.
// Yaw = (a*f - d*c)*h + (c*e - b*f)*g + (d*b - a*e)*i; Pitch = (g*a + h*b) + i*c; Roll = ((a*f - d*c)*r + (c*e - b*f)*q) + (d*b - a*e)*p. Every instruction of the 57-instruction body is accounted for.
// CONFIDENCE: MEDIUM
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
SeatSteerYawDelta = (SA * SF - SD * SC) * SH + (SC * SE - SB * SF) * SG + (SD * SB - SA * SE) * SI;
SeatSteerPitchDelta = (SG * SA + SH * SB) + SI * SC;
SeatSteerRollDelta = ((SA * SF - SD * SC) * SR + (SC * SE - SB * SF) * SQ) + (SD * SB - SA * SE) * SP;
