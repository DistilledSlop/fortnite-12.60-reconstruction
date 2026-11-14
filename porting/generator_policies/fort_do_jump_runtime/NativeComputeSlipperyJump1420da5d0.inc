// CONFIDENCE: HIGH. Complete original helper 0x1420da5d0; exact scalar SSE ordering.
FVector Result = InVelocity;
const __m128 NormalZ = _mm_set_ss(FloorImpactNormal.Z);
if (!(_mm_movemask_ps(_mm_cmpgt_ss(NormalZ, _mm_set_ss(0.0001f))) & 1))
    return Result; // jbe also takes unordered.

const __m128 X = _mm_set_ss(InVelocity.X);
const __m128 Y = _mm_set_ss(InVelocity.Y);
const __m128 Sign = _mm_set_ss(-0.0f);
__m128 Z = _mm_mul_ss(Y, _mm_set_ss(FloorImpactNormal.Y));
Z = _mm_add_ss(Z, _mm_mul_ss(X, _mm_set_ss(FloorImpactNormal.X)));
Z = _mm_add_ss(Z, _mm_mul_ss(NormalZ, _mm_set_ss(InVelocity.Z)));
Z = _mm_xor_ps(_mm_div_ss(Z, NormalZ), Sign);
Result.Z = _mm_cvtss_f32(Z);

float Boost = 20.0f; // Original 0x1478c51b8; applies when Z<0 OR unordered.
if (_mm_movemask_ps(_mm_cmpge_ss(Z, _mm_setzero_ps())) & 1)
{
    __m128 Sq = _mm_add_ss(_mm_mul_ss(X, X), _mm_mul_ss(Y, Y));
    Sq = _mm_add_ss(Sq, _mm_mul_ss(Z, Z));
    const __m128 One = _mm_set_ss(1.0f);
    // Original ucomiss + je deliberately includes unordered equality.
    if (!(_mm_movemask_ps(_mm_or_ps(_mm_cmpeq_ss(Sq, One),
        _mm_cmpunord_ss(Sq, One))) & 1))
    {
        if (!(_mm_movemask_ps(_mm_cmpge_ss(Sq, _mm_set_ss(1.e-8f))) & 1))
            Z = _mm_setzero_ps();
        else
        {
            const __m128 Half = _mm_set_ss(0.5f);
            const __m128 HalfSq = _mm_mul_ss(Sq, Half);
            __m128 Inv = _mm_rsqrt_ss(Sq);
            Inv = _mm_add_ss(Inv, _mm_mul_ss(Inv,
                _mm_sub_ss(Half, _mm_mul_ss(HalfSq, _mm_mul_ss(Inv, Inv)))));
            Inv = _mm_add_ss(Inv, _mm_mul_ss(Inv,
                _mm_sub_ss(Half, _mm_mul_ss(HalfSq, _mm_mul_ss(Inv, Inv)))));
            Z = _mm_mul_ss(Inv, Z);
        }
    }
    __m128 AngleInput = _mm_set_ss(-1.0f);
    if (_mm_movemask_ps(_mm_cmpge_ss(Z, AngleInput)) & 1)
        AngleInput = _mm_min_ss(Z, One);
    const float AngleDegrees = _mm_cvtss_f32(_mm_mul_ss(
        _mm_set_ss(::asinf(_mm_cvtss_f32(AngleInput))),
        _mm_set_ss(57.2957763671875f)));
    Boost = Player.SlipperySlopeParams.SlopeLaunchVerticalVelocityBoost
        .GetValueAtLevel(AngleDegrees, nullptr);
}
const float MaxZ = Player.SlipperySlopeParams.MaxVerticalLaunchSpeed
    .GetValueAtLevel(0.0f, nullptr);
const __m128 Sum = _mm_add_ss(_mm_set_ss(Boost), _mm_set_ss(Result.Z));
const __m128 Max = _mm_set_ss(MaxZ);
__m128 Clamped = _mm_xor_ps(Max, Sign);
if (_mm_movemask_ps(_mm_cmpge_ss(Sum, Clamped)) & 1)
    Clamped = _mm_min_ss(Sum, Max);
Result.Z = _mm_cvtss_f32(Clamped);
return Result;
