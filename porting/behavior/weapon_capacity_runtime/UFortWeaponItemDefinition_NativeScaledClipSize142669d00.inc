// CONFIDENCE: HIGH. Complete original 0x142669d00; explicit borrowed stat row binding.
// Intrinsics preserve MAXSS unordered behavior, operation order and CVTTSS2SI overflow.
__m128 Delta = _mm_sub_ss(_mm_set_ss(Level), _mm_set_ss(static_cast<float>(Stats.BaseLevel)));
Delta = _mm_max_ss(Delta, _mm_setzero_ps());
const __m128 Scale = _mm_add_ss(_mm_mul_ss(Delta, _mm_set_ss(Stats.ClipScale)), _mm_set_ss(1.0f));
return _mm_cvttss_si32(_mm_mul_ss(Scale, _mm_set_ss(static_cast<float>(Stats.ClipSize))));
