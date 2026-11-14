// Generated from world_manager_lifecycle_runtime/support.py.
#pragma once
#include "CoreMinimal.h"
#include "Delegates/Delegate.h"
#include <emmintrin.h>

// Original native single-cast delegate at AFortWorldManager +0x848.
DECLARE_DELEGATE_TwoParams(FFortNativeWorldQueryResult, bool, const FString&);

namespace FortWorldManagerOriginalGrid
{
inline int32 Mul32(int32 A, int32 B)
{
    const uint32 Bits = uint32(A) * uint32(B);
    int32 Result;
    FMemory::Memcpy(&Result, &Bits, sizeof(Result));
    return Result;
}
inline int32 ConvertAndHalf(float Value)
{
    // Native CVTSS2SI uses MXCSR rounding, followed by an arithmetic shift.
    return _mm_cvtsi128_si32(_mm_srai_epi32(_mm_cvtsi32_si128(
        _mm_cvtss_si32(_mm_set_ss(Value))), 1));
}
inline float Origin(int32 Dimension, int32 TileEdge)
{
    const float HalfProduct = float(Mul32(Dimension, TileEdge) / 2);
    return float(ConvertAndHalf((HalfProduct + HalfProduct) - 0.5f)) * -512.0f;
}
}
