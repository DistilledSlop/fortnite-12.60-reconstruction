#pragma once
#include "CoreMinimal.h"

// Source-level state contract for 12.60 native 0x141d394f0 / 0x141d398d0.
// This is NOT a claim that an arbitrary engine USignificanceManager already
// supplies these records. The owner must bind the real state listed below.
namespace FoundationViewSignificance
{
struct alignas(16) FNativeTransform
{
    float Rotation[4];
    float Translation[4];
    float Scale[4];
};
struct FPoint { float X, Y, Z; };
struct alignas(16) FCachedView
{
    float LoadRadiusScale;            // +00
    FPoint LoadProbe;                 // +04
    FPoint UnloadProbe;               // +10
    uint8 UninitializedPointPad[4];   // +1c: never read by recovered algorithms
    FNativeTransform Transform;      // +20, exact original view transform
    uint8 UninitializedTail[16];      // +50: original stores uninitialized stack
};
static_assert(sizeof(FNativeTransform) == 0x30, "native transform lanes");
static_assert(offsetof(FCachedView, Transform) == 0x20, "cache transform");
static_assert(sizeof(FCachedView) == 0x60, "native cache stride");

// Semantic input records populated by the native owner. These are not binary
// overlays for Engine objects, and must never be reached by reinterpret_cast.
struct FViewRecord
{
    uint8 Type;                      // original native view +00
    FNativeTransform Transform;      // +10, including all four padding lanes
    FPoint Position;                 // +50
    float VelocityX, VelocityY;      // +70/+74; Z is not used
};
struct FManagerState
{
    uint8 AllowsFullSignificance;    // original native manager +15a
    TArray<const FViewRecord*> Views;// ordered native view records, +1c8
};
struct FManagedBounds
{
    FPoint Min, Max;                 // original extended managed-info +b0/+bc
    const FManagerState* Manager;    // original +d0, must be valid
};
// References are deliberate: producer and callback must share the actual live
// configuration and cache object; no hard-coded defaults or private copies.
struct FNativeState
{
    const uint8& Gate;               // 1478c36ed, shared with UsesHLODStreaming
    const int32& PredictiveViews;    // 148126190; enabled iff >0
    const float& ForwardFactor;      // 1478c36b4
    const float& BackwardFactor;     // 1478c36b8
    const float& MaxLoadScale;       // 1478c36bc
    const float& HeightExponent;     // 1478c36c0
    const float& HeightCeiling;      // 1478c36c4
    const float& SpeedForMaxEffect;  // 1478c36c8
    const float& LoadRadius;         // 1478c36cc
    const float& UnloadRadius;       // 1478c36d0
    TArray<FCachedView>& Cache;      // *global pointer 1478c3c70 (not inline array)
};
float Uncached(const FNativeState&, const FManagedBounds&, const FNativeTransform&); // 141d39910
void ProduceCachedViews(const FNativeState&, const FManagerState&); // 141d394f0
float Significance(const FNativeState&, const FManagedBounds&, const FNativeTransform&); // 141d398d0
}

// Public unreflected helper contracts, deliberately separate from Engine types.
using FFortFoundationViewState = FoundationViewSignificance::FNativeState;
using FFortFoundationViewManager = FoundationViewSignificance::FManagerState;
using FFortFoundationViewBounds = FoundationViewSignificance::FManagedBounds;
using FFortFoundationViewTransform = FoundationViewSignificance::FNativeTransform;
