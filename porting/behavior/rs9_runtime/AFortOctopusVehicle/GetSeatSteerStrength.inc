// Original body: 0x1419d7be0 (thunk 0x142a08d30)
// Offset evidence: float fields at +0x12d8, +0x12dc, +0x12e0, +0x12e4, +0x12e8, +0x12ec, +0x12fc, +0x1300, +0x1304 (no reflected members in the class own region from 0xf38); scalar float expression reproduced in SSE op order.
// CONFIDENCE: HIGH
const float P1 = NativeRecoveredFloat_0x12ec;
const float P2 = NativeRecoveredFloat_0x12d8;
const float P3 = NativeRecoveredFloat_0x12e0;
const float P4 = NativeRecoveredFloat_0x12dc;
const float P5 = NativeRecoveredFloat_0x12e8;
const float P6 = NativeRecoveredFloat_0x12e4;
const float A = (P2 * P1) - (P6 * P3);
const float B = (P6 * P4) - (P2 * P5);
const float C = (P3 * P5) - (P4 * P1);
const float Strength = ((A * NativeRecoveredFloat_0x1300) + (C * NativeRecoveredFloat_0x12fc)) + (B * NativeRecoveredFloat_0x1304);
return Strength;
