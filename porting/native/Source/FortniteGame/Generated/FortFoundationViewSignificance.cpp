#include "FortFoundationViewSignificance.h"
#include <cmath>
#include <xmmintrin.h>

// Preserve the original separate SSE arithmetic operations on native Clang.
#if defined(__clang__)
#pragma clang fp contract(off)
#endif

namespace FoundationViewSignificance
{
static float DistanceSquared(const FManagedBounds& B, const FPoint& P)
{
    float D = 0.0f;
    float Edge = B.Min.X;
    if (!(P.X >= Edge) || (Edge = B.Max.X, Edge < P.X)) D = (P.X - Edge) * (P.X - Edge);
    Edge = B.Min.Y;
    if (!(P.Y >= Edge) || (Edge = B.Max.Y, Edge < P.Y)) D += (P.Y - Edge) * (P.Y - Edge);
    Edge = B.Min.Z;
    if (!(P.Z >= Edge) || (Edge = B.Max.Z, Edge < P.Z)) D += (P.Z - Edge) * (P.Z - Edge);
    return D;
}
static float Restrict(const FManagedBounds& B, float Value)
{
    return !B.Manager->AllowsFullSignificance && Value >= 2.0f ? 1.9998f : Value;
}
static bool LanesEqual(const float* A, const float* B, bool bNegateB = false)
{
    // Original CMP_LT(0.0001, abs(delta)) rejects only ordered > tolerance.
    // In particular, replacing this with <= would change NaN lane behavior.
    for (int32 I = 0; I != 4; ++I)
        if (std::fabs(bNegateB ? A[I] + B[I] : A[I] - B[I]) > 0.0001f) return false;
    return true;
}
static bool TransformEqual(const FNativeTransform& A, const FNativeTransform& B)
{
    return LanesEqual(A.Translation, B.Translation) &&
        (LanesEqual(A.Rotation, B.Rotation) || LanesEqual(A.Rotation, B.Rotation, true)) &&
        LanesEqual(A.Scale, B.Scale);
}
float Uncached(const FNativeState& S, const FManagedBounds& B, const FNativeTransform& View)
{
    const float LoadSq = S.LoadRadius * S.LoadRadius;
    const float UnloadSq = S.UnloadRadius > 0.0f ? S.UnloadRadius * S.UnloadRadius : 0.0f;
    const FPoint Point{View.Translation[0], View.Translation[1],
        S.PredictiveViews > 0 && S.HeightExponent > 0.0f && View.Translation[2] >= S.HeightCeiling
        ? S.HeightCeiling : 0.0f};
    const float D = DistanceSquared(B, Point);
    float R = 0.0f;
    if (LoadSq < UnloadSq)
    {
        R = ((D - UnloadSq) + (D - UnloadSq)) / (LoadSq - UnloadSq);
        if (!(R > 0.0f)) R = 0.0f;
    }
    else if (!(D > LoadSq)) R = D / LoadSq + 2.0f;
    return Restrict(B, R);
}
float Significance(const FNativeState& S, const FManagedBounds& B, const FNativeTransform& View)
{
    if (!S.Gate || !(S.LoadRadius > 0.0f)) return 2.0f;
    if (S.PredictiveViews <= 0) return Uncached(S, B, View);
    const FCachedView* Found = nullptr;
    for (const FCachedView& C : S.Cache)
        if (TransformEqual(C.Transform, View)) { Found = &C; break; }
    // This miss fallback is present in the original, not an approximation of
    // the cached branch: every matching cache record takes the complete path.
    if (!Found) return Uncached(S, B, View);
    const FCachedView& C = *Found;
    const float ScaledLoad = S.LoadRadius * C.LoadRadiusScale;
    const float LoadSq = ScaledLoad * ScaledLoad;
    const float UnloadSq = S.UnloadRadius > 0.0f ? S.UnloadRadius * S.UnloadRadius : 0.0f;
    const FPoint Original{C.Transform.Translation[0], C.Transform.Translation[1], C.Transform.Translation[2]};
    const float OriginalD = DistanceSquared(B, Original);
    float R = 0.0f;
    if (LoadSq < UnloadSq)
    {
        const float LoadD = DistanceSquared(B, C.LoadProbe);
        const float UnloadD = DistanceSquared(B, C.UnloadProbe);
        if (LoadSq <= LoadD)
        {
            if (!(UnloadD >= UnloadSq)) R = 1.0f;
        }
        else
        {
            R = (LoadD - UnloadSq) / (LoadSq - UnloadSq);
            R = R + R;
        }
    }
    else if (!(OriginalD > LoadSq)) R = OriginalD / LoadSq + 2.0f;
    return Restrict(B, R);
}
static float ClampNative(float Value, float Upper)
{
    // COMISS/JNC followed by MINSS: unordered first comparison selects zero.
    if (!(Value >= 0.0f)) return 0.0f;
    return Value < Upper ? Value : Upper;
}
void ProduceCachedViews(const FNativeState& S, const FManagerState& Manager)
{
    // Both early exits retain the previous array, including count and records.
    if (S.PredictiveViews <= 0 || Manager.Views.Num() == 0) return;
    TArray<const FViewRecord*> Selected;
    for (const FViewRecord* View : Manager.Views)
        if (View->Type == 3) Selected.Add(View);
    S.Cache.Reset(); // original sets Num=0 without shrinking or destroying data
    S.Cache.Reserve(Selected.Num());
    for (const FViewRecord* View : Selected)
    {
        float X = View->VelocityX, Y = View->VelocityY;
        const float LengthSq = X * X + Y * Y;
        const float Length = std::sqrt(LengthSq);
        if (LengthSq > 1.0e-8f)
        {
            // Match original SSE rsqrt estimate plus its two Newton steps.
            float Inv = _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(LengthSq)));
            const float HalfLengthSq = LengthSq * 0.5f;
            Inv = Inv + Inv * (0.5f - HalfLengthSq * (Inv * Inv));
            Inv = Inv + Inv * (0.5f - HalfLengthSq * (Inv * Inv));
            X *= Inv;
            Y *= Inv;
        }
        const float Speed = ClampNative(Length / S.SpeedForMaxEffect, 1.0f);
        const float Forward = S.UnloadRadius * Speed * S.ForwardFactor;
        const float UnloadX = X * Forward + View->Position.X;
        const float UnloadY = Y * Forward + View->Position.Y;
        const float Scale = (S.MaxLoadScale - 1.0f) * Speed + 1.0f;
        const float RadiusDelta = S.UnloadRadius - Scale * S.LoadRadius;
        const float Backward = S.BackwardFactor * Speed;
        const float LoadX = UnloadX - (X * RadiusDelta) * Backward;
        const float LoadY = UnloadY - (Y * RadiusDelta) * Backward;
        const float HeightDelta = ClampNative(S.HeightCeiling - View->Position.Z, S.HeightCeiling);
        float Height = 0.0f;
        if (S.HeightExponent > 0.0f)
        {
            Height = View->Position.Z - std::pow(HeightDelta, S.HeightExponent);
            // Original MAXSS selects zero for unordered and nonpositive values.
            if (!(Height > 0.0f)) Height = 0.0f;
        }
        FCachedView& C = S.Cache[S.Cache.AddUninitialized()];
        C.LoadRadiusScale = Scale;
        C.LoadProbe = {LoadX, LoadY, Height};
        C.UnloadProbe = {UnloadX, UnloadY, Height};
        C.Transform = View->Transform;
        // +1c and +50 are uninitialized stack bytes in the original and are
        // never inputs to this producer/callback. Leave these opaque pads alone.
    }
}
}
