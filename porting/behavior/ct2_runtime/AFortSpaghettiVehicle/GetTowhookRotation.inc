// Behaviour snippet: AFortSpaghettiVehicle::GetTowhookRotation (body only).
// Original body address: 0x1419d99e0 (thunk 0x142a09040). Result written through the hidden return pointer as FRotator{Pitch, Yaw, 0}.
// Offset evidence: bInterp false reads FVector at this+0x1578, true reads this+0x1584 (CMOVZ on TEST R8B). Both are unreflected gaps (member_field NativeRecoveredFVector_0x1578 / _0x1584).
// Up vector components (0,0,1) come from writable snapshot globals 0x147c31cb0/0x147c31cb4/0x147c31cb8 (FVector::UpVector-style constants). Pitch = asin(dot) and Yaw = atan2 of the horizontal projection, per the SSE sequence.
// Constants 0x145e291b4..0x145e291d4 are .rdata polynomial coefficients of the acos approximation; COMISS/JC selects the sign branch on dot < 0.
// CONFIDENCE: MEDIUM
static_assert(sizeof(UFortGameplayAbility) > 0, "UFortGameplayAbility must be complete for the static Atan2 helper");
const FVector Dir = bInterp ? NativeRecoveredFVector_0x1584 : NativeRecoveredFVector_0x1578;
const float UpX = 0.0f;
const float UpY = 0.0f;
const float UpZ = 1.0f;
const float HalfPi = 1.570796251296997f;
const float Degrees = 57.2957763671875f;

const float S = ((Dir.X * UpX) + (UpY * Dir.Y)) + (UpZ * Dir.Z);
const float AbsS = fabsf(S);
const float OneMinusAbs = 1.0f - AbsS;
const float ClampedOneMinus = (OneMinusAbs > 0.0f) ? OneMinusAbs : 0.0f;
float Poly = 0.006670089904218912f - (AbsS * 0.0012624911032617092f);
Poly *= AbsS;
Poly -= 0.01708812639117241f;
Poly *= AbsS;
Poly += 0.03089188039302826f;
Poly *= AbsS;
Poly -= 0.050174303352832794f;
Poly *= AbsS;
Poly += 0.08897899091243744f;
Poly *= AbsS;
Poly -= 0.21459880471229553f;
Poly *= AbsS;
Poly += HalfPi;
const float AcosAbs = sqrtf(ClampedOneMinus) * Poly;
const float Elevation = (S < 0.0f) ? (AcosAbs - HalfPi) : (HalfPi - AcosAbs);

const float ProjX = Dir.X - (UpX * S);
const float ProjY = Dir.Y - (UpY * S);
const float YawRadians = UFortGameplayAbility::NativeAtan2Approx142bbd380(ProjY, ProjX);

return FRotator(Elevation * Degrees, YawRadians * Degrees, 0.0f);
