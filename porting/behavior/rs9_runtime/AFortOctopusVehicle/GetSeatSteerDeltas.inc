// Original body: 0x1419d7ab0 (thunk 0x142a08bf0)
// Offset evidence: float fields at +0x1244/+0x1248 are not used here; inputs are floats at +0x126c, +0x1270, +0x1274 (bit-copied int read used as float), +0x12d8, +0x12dc, +0x12e0, +0x12e4, +0x12e8, +0x12ec, +0x12fc, +0x1300, +0x1304 (no reflected members in the class own region from 0xf38). Outputs: Yaw (RDX), Pitch (R8), Roll (R9). Every scalar SSE op is reproduced in order.
// CONFIDENCE: HIGH
const float P1 = NativeRecoveredFVector_0x12e4.Z;
const float P2 = NativeRecoveredFVector_0x12d8.X;
const float P3 = NativeRecoveredFVector_0x12d8.Z;
const float P4 = NativeRecoveredFVector_0x12d8.Y;
const float P5 = NativeRecoveredFVector_0x12e4.Y;
const float P6 = NativeRecoveredFVector_0x12e4.X;
const float F1274 = NativeRecoveredFloat_0x1274;
const float F126C = NativeRecoveredFloat_0x126c;
const float F1270 = NativeRecoveredFloat_0x1270;
const float F12FC = NativeRecoveredFloat_0x12fc;
const float F1300 = NativeRecoveredFloat_0x1300;
const float F1304 = NativeRecoveredFloat_0x1304;
const float S7 = (P2 * P1) - (P6 * P3);
const float S8 = (P3 * P5) - (P4 * P1);
const float X6 = (P6 * P4) - (P2 * P5);
SeatSteerYawDelta = ((S7 * F1300) + (S8 * F12FC)) + (X6 * F1304);
SeatSteerPitchDelta = ((F1300 * P4) + (F12FC * P2)) + (F1304 * P3);
SeatSteerRollDelta = ((S7 * F1270) + (S8 * F126C)) + (X6 * F1274);
