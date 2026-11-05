#pragma once

#include "CoreMinimal.h"

#if PLATFORM_LINUX && PLATFORM_CPU_X86_FAMILY && !UE_SERVER
#include "LegacyOriginalUniformData.h"

// Explicit values read by original helper 0x143579e10. The view fields' upstream
// identities are not established by this type. No current-name aliases or CVar
// reads are inferred; the caller supplies the corresponding original inputs.
struct FLegacySkyAtmosphereDepthInputs
{
    float DepthResolution;
    float DepthExtent;
    float StartDepth;
    float ViewFloatD30;
    // Original View+0x4e20: allocation label identifies the aerial perspective volume.
    bool bAerialPerspectiveVolumeNonNull;
};

// Original six-float helper output, in original offset order. Its caller copies
// all six fields into View at +0xadc through +0xaf0.
struct FLegacySkyAtmosphereDepthParameters
{
    float StartDepth;
    float DepthResolution;
    float DepthResolutionInv;
    float DepthSliceLength;
    float DepthSliceLengthInv;
    float ApplyCameraAerialPerspectiveVolume;
};

// Direct x86 scalar-SSE lift, including CVTTSS2SI overflow/NaN behavior, MAXSS
// operand order, float32 division/multiplication and the calling thread's MXCSR.
// This computes values only; source acquisition, View overrides and renderer
// integration remain the caller's responsibility. No current metadata is used.
FLegacySkyAtmosphereDepthParameters ComputeLegacySkyAtmosphereDepthParameters(
    const FLegacySkyAtmosphereDepthInputs& Inputs);

// Produce complete typed overrides for the eight original active-atmosphere
// View members at +0xad4..+0xaf0. Radii must be the original proxy values;
// this does not convert current Km fields or select the active/fallback branch.
// Contract must come from the independently pinned original View metadata.
// On success, pass these values to BuildLegacyOriginalUniformData (or the draw
// binding bridge); other missing View members still require their own producers.
// OutValues is unchanged on failure and replaced on success. No RHI work occurs.
bool BuildLegacySkyAtmosphereViewMemberValues(const FLegacyLocalUniformContract& Contract,
    float OriginalBottomRadius, float OriginalTopRadius,
    const FLegacySkyAtmosphereDepthInputs& Inputs,
    TArray<FLegacyOriginalUniformMemberValue>& OutValues, FString& Error);
#endif
