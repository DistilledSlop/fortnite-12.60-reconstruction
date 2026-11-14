// Original body: 0x1419d7be0 (thunk 0x142a08d30)
// Offset evidence: float fields at +0x12d8, +0x12dc, +0x12e0, +0x12e4, +0x12e8, +0x12ec, +0x12fc, +0x1300, +0x1304 (no reflected members in the class own region from 0xf38); scalar float expression reproduced in SSE op order.
// CONFIDENCE: HIGH
const float P1 = NativeRecoveredFVector_0x12e4.Z;
const float P2 = NativeRecoveredFVector_0x12d8.X;
const float P3 = NativeRecoveredFVector_0x12d8.Z;
const float P4 = NativeRecoveredFVector_0x12d8.Y;
const float P5 = NativeRecoveredFVector_0x12e4.Y;
const float P6 = NativeRecoveredFVector_0x12e4.X;
const float A = (P2 * P1) - (P6 * P3);
const float B = (P6 * P4) - (P2 * P5);
const float C = (P3 * P5) - (P4 * P1);
const float Strength = ((A * NativeRecoveredFloat_0x1300) + (C * NativeRecoveredFloat_0x12fc)) + (B * NativeRecoveredFloat_0x1304);
return Strength;
