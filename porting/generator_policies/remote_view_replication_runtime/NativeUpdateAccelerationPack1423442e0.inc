// CONFIDENCE: high
// Original 0x1423442e0. x86 SSE scalar operations retain CVTSS2SI's MXCSR
// rounding, indefinite result, unordered comparisons, and each float rounding.
const FVector Accel = GetCharacterMovement()->GetCurrentAcceleration();
const __m128 Zero = _mm_setzero_ps();
const __m128 One = _mm_set_ss(1.0f);
const auto Unordered = [](__m128 A, __m128 B) { return (_mm_movemask_ps(_mm_cmpunord_ss(A, B)) & 1) != 0; };
const auto EqualOrUnordered = [&](const __m128 A, const __m128 B) { return Unordered(A, B) || _mm_ucomieq_ss(A, B); };
const auto LessOrUnordered = [&](const __m128 A, const __m128 B) { return Unordered(A, B) || _mm_comilt_ss(A, B); };
const __m128 Half = _mm_set_ss(0.5f);
const __m128 X = _mm_set_ss(Accel.X);
const __m128 Y = _mm_set_ss(Accel.Y);
const __m128 Z = _mm_set_ss(Accel.Z);
const __m128 AbsMask = _mm_castsi128_ps(_mm_set1_epi32(0x7fffffff));
const __m128 SignMask = _mm_castsi128_ps(_mm_set1_epi32(static_cast<int32>(0x80000000u)));
const __m128 AbsX = _mm_and_ps(X, AbsMask);
const __m128 AbsY = _mm_and_ps(Y, AbsMask);
const bool bYDominant = _mm_comigt_ss(AbsY, AbsX) != 0;
const __m128 Maximum = bYDominant ? AbsY : AbsX;
const __m128 Minimum = bYDominant ? AbsX : AbsY;
__m128 Angle = Zero;
// UCOMISS/JZ accepts both equal and unordered.
if (!EqualOrUnordered(Maximum, Zero))
{
    const __m128 Ratio = _mm_div_ss(Minimum, Maximum);
    const __m128 Square = _mm_mul_ss(Ratio, Ratio);
    __m128 Poly = _mm_mul_ss(Square, _mm_set_ss(0.0072128851898014545f));
    Poly = _mm_sub_ss(Poly, _mm_set_ss(0.03505967929959297f));
    Poly = _mm_mul_ss(Poly, Square);
    Poly = _mm_add_ss(Poly, _mm_set_ss(0.08167587965726852f));
    Poly = _mm_mul_ss(Poly, Square);
    Poly = _mm_sub_ss(Poly, _mm_set_ss(0.13374657928943634f));
    Poly = _mm_mul_ss(Poly, Square);
    Poly = _mm_add_ss(Poly, _mm_set_ss(0.19856563210487366f));
    Poly = _mm_mul_ss(Poly, Square);
    Poly = _mm_sub_ss(Poly, _mm_set_ss(0.3332499861717224f));
    Poly = _mm_mul_ss(Poly, Square);
    Poly = _mm_add_ss(Poly, One);
    Angle = _mm_mul_ss(Ratio, Poly);
    if (bYDominant) Angle = _mm_sub_ss(_mm_set_ss(1.5707963705062866f), Angle);
    // COMISS/JNC falls through for both less-than and unordered.
    if (LessOrUnordered(X, Zero)) Angle = _mm_sub_ss(_mm_set_ss(3.1415927410125732f), Angle);
    if (LessOrUnordered(Y, Zero)) Angle = _mm_xor_ps(Angle, SignMask);
}
__m128 Direction = _mm_mul_ss(Angle, _mm_set_ss(40.58450698852539f));
Direction = _mm_sub_ss(_mm_add_ss(Direction, Direction), Half);
const int32 DirectionInt = _mm_cvtss_si32(Direction) >> 1;
__m128 Magnitude = _mm_sqrt_ss(_mm_add_ss(_mm_mul_ss(X, X), _mm_mul_ss(Y, Y)));
int32 MagnitudeInt = 0;
if (!EqualOrUnordered(Magnitude, Zero))
{
    Magnitude = _mm_mul_ss(Magnitude, _mm_set_ss(0.000244140625f));
    Magnitude = LessOrUnordered(Magnitude, Zero) ? Zero : _mm_min_ss(Magnitude, One);
    Magnitude = _mm_mul_ss(Magnitude, _mm_set_ss(254.0f));
    Magnitude = _mm_sub_ss(_mm_add_ss(Magnitude, Magnitude), Half);
    MagnitudeInt = (_mm_cvtss_si32(Magnitude) >> 1) + 1;
}
AccelerationPack = static_cast<uint16>((static_cast<uint32>(MagnitudeInt) << 8) | (static_cast<uint32>(DirectionInt) & 0xffu));
AccelerationZPack = 0;
if (bLocalIsSkydiving && !EqualOrUnordered(Z, Zero))
{
    __m128 Vertical = _mm_sub_ss(Z, _mm_set_ss(-4096.0f));
    Vertical = _mm_mul_ss(Vertical, _mm_set_ss(0.0001220703125f));
    Vertical = LessOrUnordered(Vertical, Zero) ? Zero : _mm_min_ss(Vertical, One);
    Vertical = _mm_sub_ss(_mm_mul_ss(Vertical, _mm_set_ss(254.0f)), _mm_set_ss(127.0f));
    Vertical = _mm_add_ss(_mm_add_ss(Vertical, Vertical), Half);
    AccelerationZPack = static_cast<int8>(_mm_cvtss_si32(Vertical) >> 1);
}
