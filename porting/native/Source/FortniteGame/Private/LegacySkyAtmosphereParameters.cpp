#include "LegacySkyAtmosphereParameters.h"

#if PLATFORM_LINUX && PLATFORM_CPU_X86_FAMILY && !UE_SERVER
#include <emmintrin.h>

static_assert(sizeof(FLegacySkyAtmosphereDepthParameters) == 24 &&
    STRUCT_OFFSET(FLegacySkyAtmosphereDepthParameters, StartDepth) == 0 &&
    STRUCT_OFFSET(FLegacySkyAtmosphereDepthParameters, DepthResolution) == 4 &&
    STRUCT_OFFSET(FLegacySkyAtmosphereDepthParameters, DepthResolutionInv) == 8 &&
    STRUCT_OFFSET(FLegacySkyAtmosphereDepthParameters, DepthSliceLength) == 12 &&
    STRUCT_OFFSET(FLegacySkyAtmosphereDepthParameters, DepthSliceLengthInv) == 16 &&
    STRUCT_OFFSET(FLegacySkyAtmosphereDepthParameters, ViewPointerFlag) == 20,
    "original atmosphere helper has six consecutive float32 outputs");

// Original 0x143579e10..0x143579eb2; the pinned instruction evidence is in
// behavior/view_missing_producer_runtime/evidence.json. Intrinsics preserve the
// original scalar operations instead of substituting current FMath formulas.
FLegacySkyAtmosphereDepthParameters ComputeLegacySkyAtmosphereDepthParameters(
    const FLegacySkyAtmosphereDepthInputs& Inputs)
{
    FLegacySkyAtmosphereDepthParameters Result;
    int32 Resolution = _mm_cvtt_ss2si(_mm_set_ss(Inputs.DepthResolution));
    if (Resolution < 4) Resolution = 4;
    const __m128 ResolutionValue = _mm_cvtepi32_ps(_mm_cvtsi32_si128(Resolution));
    const __m128 One = _mm_set_ss(1.0f);
    const __m128 Extent = _mm_max_ss(_mm_set_ss(Inputs.DepthExtent), One);
    const __m128 SliceLength = _mm_div_ss(Extent, ResolutionValue);
    _mm_store_ss(&Result.ViewPointerFlag, Inputs.bViewPointer4E20NonNull ? One : _mm_setzero_ps());
    _mm_store_ss(&Result.DepthResolution, ResolutionValue);
    _mm_store_ss(&Result.DepthResolutionInv, _mm_div_ss(One, ResolutionValue));
    _mm_store_ss(&Result.DepthSliceLength, SliceLength);
    _mm_store_ss(&Result.DepthSliceLengthInv, _mm_div_ss(One, SliceLength));
    __m128 Start = _mm_max_ss(_mm_set_ss(Inputs.StartDepth), _mm_setzero_ps());
    // Exact original constant bits: 0x47c35000 and 0x3727c5ac.
    const __m128 ToLargeUnits = _mm_castsi128_ps(_mm_cvtsi32_si128(0x47c35000));
    const __m128 ToSmallUnits = _mm_castsi128_ps(_mm_cvtsi32_si128(0x3727c5ac));
    Start = _mm_mul_ss(Start, ToLargeUnits);
    Start = _mm_max_ss(Start, _mm_set_ss(Inputs.ViewFloatD30));
    Start = _mm_mul_ss(Start, ToSmallUnits);
    _mm_store_ss(&Result.StartDepth, Start);
    return Result;
}
#endif
