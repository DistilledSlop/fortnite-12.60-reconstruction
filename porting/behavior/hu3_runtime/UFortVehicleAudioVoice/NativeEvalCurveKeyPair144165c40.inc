// Helper 0x144165c40, host UFortVehicleAudioVoice (21 callers).
// Offset evidence: KeyA/KeyB are 0x1c-stride records (caller 0x1441655c0 at CALL 0x144165c40 passes IMUL ...,0x1c element pointers).
//   KeyA/KeyB +0x04 float Time, +0x08 float Value; KeyA +0x00 uint8 Mode, KeyA +0x14 float LeaveTangent; KeyB +0x0c float ArriveTangent.
// Constant [0x145a38ac0] = 0x3eaaaaab = 0.33333334f (read from PE .rdata). Scalar SSE only; every asm instruction is transcribed.
// Mode 0 is linear; Mode 1 holds KeyA.Value; other Modes use the asm cubic path (control points P0, P0+Q1, P1-Q2, P1).
// CONFIDENCE: MEDIUM
const uint8 CurveMode = *KeyA;
const float CurveATime = *reinterpret_cast<const float*>(KeyA + 0x04);
const float CurveAValue = *reinterpret_cast<const float*>(KeyA + 0x08);
const float CurveALeave = *reinterpret_cast<const float*>(KeyA + 0x14);
const float CurveBTime = *reinterpret_cast<const float*>(KeyB + 0x04);
const float CurveBValue = *reinterpret_cast<const float*>(KeyB + 0x08);
const float CurveBArrive = *reinterpret_cast<const float*>(KeyB + 0x0c);
const float CurveDt = CurveBTime - CurveATime;
if (!(CurveDt > 0.0f)) { return CurveAValue; }
const float CurveS = (EvalTime - CurveATime) / CurveDt;
if (CurveMode == 1) { return CurveAValue; }
if (CurveMode == 0) { return CurveAValue + (CurveBValue - CurveAValue) * CurveS; }
const float CurveK3 = 0.33333334f;
float CurveQ1 = (CurveDt * CurveALeave) * CurveK3;
float CurveQ2 = (CurveDt * CurveBArrive) * CurveK3;
float CurveX4 = CurveQ1 + CurveAValue;
float CurveX0 = CurveQ1 * CurveS + CurveAValue;
float CurveX6 = CurveBValue - CurveQ2;
float CurveX5 = CurveQ2 * CurveS + CurveX6;
float CurveX2 = CurveX6 - CurveX4;
float CurveX1 = CurveX2 * CurveS + CurveX4;
CurveX5 = CurveX5 - CurveX1;
float CurveX3 = CurveX1 - CurveX0;
CurveX5 = CurveX5 + CurveX2;
CurveX3 = CurveX3 + CurveQ1;
CurveX5 = CurveX5 * CurveS;
CurveX1 = CurveX3 * CurveS + CurveAValue;
CurveX5 = CurveX5 + CurveX4;
CurveX5 = CurveX5 - CurveX1;
CurveX5 = CurveX5 + CurveX3;
CurveX5 = CurveX5 * CurveS;
return CurveX5 + CurveAValue;
