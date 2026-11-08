// Body 0x1419d99e0 (thunk 0x142a09040). Offset evidence: FVector at this+0x1578 (bInterp false) or this+0x1584 (bInterp true), named NativeRecoveredVector_0x1578 / _0x1584. The body dots it with the writable (0,0,1) up-vector snapshot at 0x147c31cb0..0x147c31cb8 (0, 0, 1 values, written as literals). Pitch is asin(S) from the polynomial acos approximation; yaw is Atan2Approx(Y, X) of the horizontal projection; Roll is 0.
// CONFIDENCE: MEDIUM
static_assert(sizeof(UFortGameplayAbility) > 0, "UFortGameplayAbility must be declared to call the ported Atan2 helper");
const FVector& V = bInterp ? NativeRecoveredVector_0x1584 : NativeRecoveredVector_0x1578;
const float LocalS = V.X * 0.0f + 0.0f * V.Y + 1.0f * V.Z;
const float LocalA = FMath::Abs(LocalS);
const float LocalOneMinusA = 1.0f - LocalA;
const float LocalQ = FMath::Sqrt(LocalOneMinusA > 0.0f ? LocalOneMinusA : 0.0f);
float LocalPoly = 0.0066700899f - LocalA * 0.0012624911f;
LocalPoly = LocalPoly * LocalA - 0.0170881264f;
LocalPoly = LocalPoly * LocalA + 0.0308918804f;
LocalPoly = LocalPoly * LocalA - 0.0501743034f;
LocalPoly = LocalPoly * LocalA + 0.0889790909f;
LocalPoly = LocalPoly * LocalA - 0.2145988047f;
LocalPoly = LocalPoly * LocalA + 1.570796251296997f;
const float LocalAcosA = LocalQ * LocalPoly;
const float LocalPitch = LocalS < 0.0f ? (LocalAcosA - 1.570796251296997f) : (1.570796251296997f - LocalAcosA);
const float LocalYaw = UFortGameplayAbility::NativeAtan2Approx142bbd380(V.Y - 0.0f * LocalS, V.X - 0.0f * LocalS);
FRotator Result;
Result.Pitch = LocalPitch * 57.2957763671875f;
Result.Yaw = LocalYaw * 57.2957763671875f;
Result.Roll = 0.0f;
return Result;
