#pragma once

#include "CoreMinimal.h"

#if PLATFORM_LINUX && PLATFORM_CPU_X86_FAMILY && !UE_SERVER

// Explicit values read by original helper 0x143579e10. The view fields' upstream
// identities are not established by this type. No current-name aliases or CVar
// reads are inferred; the caller supplies the corresponding original inputs.
struct FLegacySkyAtmosphereDepthInputs
{
    float DepthResolution;
    float DepthExtent;
    float StartDepth;
    float ViewFloatD30;
    bool bViewPointer4E20NonNull;
};

// Original six-float helper output, in original offset order. The first five
// fields are copied into View at +0xadc through +0xaec by its caller.
struct FLegacySkyAtmosphereDepthParameters
{
    float StartDepth;
    float DepthResolution;
    float DepthResolutionInv;
    float DepthSliceLength;
    float DepthSliceLengthInv;
    float ViewPointerFlag;
};

// Direct x86 scalar-SSE lift, including CVTTSS2SI overflow/NaN behavior, MAXSS
// operand order, float32 division/multiplication and the calling thread's MXCSR.
// This computes values only; source acquisition, View overrides and renderer
// integration remain the caller's responsibility. No current metadata is used.
FLegacySkyAtmosphereDepthParameters ComputeLegacySkyAtmosphereDepthParameters(
    const FLegacySkyAtmosphereDepthInputs& Inputs);
#endif
