#include "LegacySkyAtmosphereParameters.h"

#if PLATFORM_LINUX && PLATFORM_CPU_X86_FAMILY && !UE_SERVER
#include <emmintrin.h>

static_assert(sizeof(FLegacySkyAtmosphereDepthParameters) == 24 &&
    STRUCT_OFFSET(FLegacySkyAtmosphereDepthParameters, StartDepth) == 0 &&
    STRUCT_OFFSET(FLegacySkyAtmosphereDepthParameters, DepthResolution) == 4 &&
    STRUCT_OFFSET(FLegacySkyAtmosphereDepthParameters, DepthResolutionInv) == 8 &&
    STRUCT_OFFSET(FLegacySkyAtmosphereDepthParameters, DepthSliceLength) == 12 &&
    STRUCT_OFFSET(FLegacySkyAtmosphereDepthParameters, DepthSliceLengthInv) == 16 &&
    STRUCT_OFFSET(FLegacySkyAtmosphereDepthParameters, ApplyCameraAerialPerspectiveVolume) == 20,
    "original atmosphere helper has six consecutive float32 outputs");

// Original 0x143579e10..0x143579eb2; the pinned instruction evidence is in
// material_pipeline/evidence/view_missing_producer_runtime/evidence.json. Intrinsics preserve the
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
    _mm_store_ss(&Result.ApplyCameraAerialPerspectiveVolume, Inputs.bAerialPerspectiveVolumeNonNull ? One : _mm_setzero_ps());
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

bool BuildLegacySkyAtmosphereViewMemberValues(const FLegacyLocalUniformContract& Contract,
    bool bRenderSkyAtmosphere, float OriginalBottomRadius, float OriginalTopRadius,
    const FLegacySkyAtmosphereDepthInputs& Inputs,
    TArray<FLegacyOriginalUniformMemberValue>& OutValues, FString& Error)
{
    if (Contract.ParameterName != TEXT("View") ||
        Contract.OriginalStructTypeName != TEXT("FViewUniformShaderParameters") ||
        Contract.OriginalSize != 3952)
    {
        Error = TEXT("atmosphere values require the original View contract");
        return false;
    }
    static const TCHAR* Names[] = {
        TEXT("SkyAtmosphereBottomRadius"),
        TEXT("SkyAtmosphereTopRadius"),
        TEXT("SkyAtmosphereAerialPerspectiveStartDepth"),
        TEXT("SkyAtmosphereCameraAerialPerspectiveVolumeDepthResolution"),
        TEXT("SkyAtmosphereCameraAerialPerspectiveVolumeDepthResolutionInv"),
        TEXT("SkyAtmosphereCameraAerialPerspectiveVolumeDepthSliceLength"),
        TEXT("SkyAtmosphereCameraAerialPerspectiveVolumeDepthSliceLengthInv"),
        TEXT("SkyAtmosphereApplyCameraAerialPerspectiveVolume")
    };
    // Validate the entire requested metadata subset before evaluating the helper.
    // Copy original leaf types, never current metadata or guessed field aliases.
    TArray<FLegacyOriginalUniformMemberValue> Values;
    Values.Reserve(UE_ARRAY_COUNT(Names));
    for (uint32 Index = 0; Index < UE_ARRAY_COUNT(Names); ++Index)
    {
        const FLegacyLocalMemberContract* Found = nullptr;
        for (const auto& Member : Contract.OriginalMembers)
        {
            if (Member.Name != Names[Index]) continue;
            if (Found)
            {
                Error = TEXT("duplicate original atmosphere View member");
                return false;
            }
            Found = &Member;
        }
        if (!Found || Found->Offset != 0xad4 + Index * sizeof(float) ||
            Found->BaseType != UBMT_FLOAT32 || Found->Precision != EShaderPrecisionModifier::Float ||
            Found->Rows != 1 || Found->Columns != 1 || Found->Elements ||
            Found->StructSize || Found->Members.Num() ||
            !Found->ShaderType.IsEmpty() || !Found->StructTypeName.IsEmpty())
        {
            Error = FString(TEXT("original atmosphere View scalar contract mismatch: ")) + Names[Index];
            return false;
        }
        auto& Value = Values.AddDefaulted_GetRef();
        Value.Path = FString(TEXT("View.")) + Names[Index];
        Value.Type = *Found;
        Value.Bytes.SetNumUninitialized(sizeof(float));
    }
    if (bRenderSkyAtmosphere)
    {
        const FLegacySkyAtmosphereDepthParameters Depth = ComputeLegacySkyAtmosphereDepthParameters(Inputs);
        // Original caller 0x1434eff1e..0x1434eff63 copies two radius words and all
        // six helper words. Preserve those bytes, including NaN payloads.
        FMemory::Memcpy(Values[0].Bytes.GetData(), &OriginalBottomRadius, sizeof(float));
        FMemory::Memcpy(Values[1].Bytes.GetData(), &OriginalTopRadius, sizeof(float));
        for (uint32 Index = 0; Index < 6; ++Index)
            FMemory::Memcpy(Values[Index + 2].Bytes.GetData(),
                reinterpret_cast<const uint8*>(&Depth) + Index * sizeof(float), sizeof(float));
    }
    else
    {
        // Original 0x1434f0234..0x1434f0290, including the apply-volume flag.
        const float DisabledValue = 1.0f;
        for (auto& Value : Values)
            FMemory::Memcpy(Value.Bytes.GetData(), &DisabledValue, sizeof(float));
    }
    OutValues = MoveTemp(Values);
    Error.Reset();
    return true;
}
#endif
