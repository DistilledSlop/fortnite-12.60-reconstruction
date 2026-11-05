// DRAFT: a typed, reviewable lift of 0x141ddfad0, not a compiled/native port.
// Read evidence.json before integration. Hooks are required effects, never no-op fallbacks.
// Includes assume Generated/ is an include root in a future integration translation unit.
#pragma once
#include "AFortWindManager.h"
#include "ABuildingSMActor.h"
#include "Curves/CurveLinearColor.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace FortWindSamplingDraft
{
// NativeWindUpdateBudget_0x220 is integrated with constructor default 256.
struct FState
{
    // Manager global vector is already NativeWindVector_0x430; this is not viewer location.
};

// Explicit integration contracts. The names after '_' identify original callees/effects.
// Their implementations must preserve logging, delegates, spatial storage and UID effects.
struct FEffects
{
    virtual ~FEffects() = default;
    virtual bool WindEnabled_14812a5f8() = 0;
    virtual bool IsDedicatedServer_143f7a920() = 0;
    virtual void LogAndDestroyDedicated_143f6c200(AFortWindManager&) = 0;
    virtual bool GetWorldTimeOrLogNull(AFortWindManager&, float& WorldTime_0x548) = 0;
    virtual void BeginStat(const TCHAR*) = 0;
    virtual void EndStat() = 0;
    // GUObjectArray item flag 0x20000000 (not an unconstrained UObject IsValid replacement).
    virtual bool ActorHasNativeInvalidFlag(ABuildingSMActor*) = 0;
    virtual FVector ActorRootLocationOrZero(ABuildingSMActor*) = 0;
    virtual bool GetCylinder_141dcf3f0(AFortWindManager&, const FFortWindImpulseHandle&,
        FFortWindImpulseCylinder&) = 0;
    virtual void SetupAnimatingMaterials_141ddae00(AFortWindManager&, FFortWindResponder&) = 0;
    virtual void SpatialRemove_141dd61e0(AFortWindManager&, int32, const FBox&) = 0;
    virtual void SpatialInsert_141dd0bb0(AFortWindManager&, int32, const FBox&, const TCHAR*) = 0;
    // 141dd9880 resets time/current magnitude and advances radial section, clipping boxes.
    virtual bool AdvanceDeltaSection_141dd9880(FFortWindImpulseCylinderDelta&) = 0;
    // Integrated as NativeBuildWindSectionBoxes: appends OuterWorldBounds minus InnerWorldBounds.
    virtual void BuildSectionBoxes_141dcb5d0(const FFortWindImpulseCylinderDelta&, TArray<FBox>&) = 0;
    // Integrated as NativeStartQueuedWindDelta.
    virtual void StartNextDelta_141ddef30(AFortWindManager&) = 0;
    // Both dispatch virtual0x210 and consume a UID for a by-value cylinder exec temporary.
    // Sampling assembly passes the cylinder in RDX at both death call sites.
    // Restored events are OnWindImpulseCylinderDeltaComplete and OnWindImpulseCylinderDeath.
    virtual void CylinderChanged_142b54060(AFortWindManager&, const FFortWindImpulseCylinder&) = 0;
    virtual void CylinderRemoved_142b53fa0(AFortWindManager&, const FFortWindImpulseCylinder&) = 0;
    virtual void SetScalarByIndex_14433ac70(UMaterialInstanceDynamic*, int32, float) = 0;
    virtual void SetVectorByIndex_14433b1f0(UMaterialInstanceDynamic*, int32, const FLinearColor&) = 0;
};

inline bool InBoundsInclusive(const FBox& B, const FVector& P)
{
    // Native code compares coordinates, ignoring FBox::IsValid.
    return B.Min.X <= P.X && P.X <= B.Max.X && B.Min.Y <= P.Y && P.Y <= B.Max.Y
        && B.Min.Z <= P.Z && P.Z <= B.Max.Z;
}

// Keep the capture's short-circuit topology. Two minimum-coordinate tests are
// conditional on another coordinate equalling its maximum. This is unusual and
// must be checked against machine code before replacing it with a standard box test.
inline bool InRadiusCaptureBounds(const FBox& B, const FVector& P)
{
    return B.Min.X <= P.X && (P.X < B.Max.X || P.X == B.Max.X) && B.Min.Y <= P.Y
        && (P.Y < B.Max.Y || (P.Y == B.Max.Y && B.Min.Z <= P.Z))
        && (P.Z < B.Max.Z || P.Z == B.Max.Z);
}
inline bool InCylinderCaptureBounds(const FBox& B, const FVector& P)
{
    return B.Min.X <= P.X && (P.X < B.Max.X || (P.X == B.Max.X && B.Min.Y <= P.Y))
        && (P.Y < B.Max.Y || P.Y == B.Max.Y) && B.Min.Z <= P.Z
        && (P.Z < B.Max.Z || P.Z == B.Max.Z);
}

inline FVector CylinderDirectionWithFalloff(const FFortWindImpulseCylinder& C, const FVector& P)
{
    if (C.OuterRadius <= 0.0f) return FVector::ZeroVector;
    const float DX = P.X - C.Location.X, DY = P.Y - C.Location.Y;
    const float DistanceSquared = DX * DX + DY * DY;
    const float Distance = FMath::Sqrt(DistanceSquared);
    if (Distance >= C.OuterRadius) return FVector::ZeroVector;
    FVector Direction = C.WindDirection;
    if (Direction == FVector::ZeroVector)
    {
        // Native uses rsqrtss plus two Newton refinements at squared threshold 1e-8.
        // Scalar sqrt is a semantic draft; numeric/NaN parity is not closed.
        Direction = DistanceSquared <= 1.e-8f ? FVector::ZeroVector
            : FVector(DX / Distance, DY / Distance, 0.0f);
    }
    if (C.InnerRadius < Distance)
        Direction *= 1.0f - (Distance - C.InnerRadius) / (C.OuterRadius - C.InnerRadius);
    return Direction;
}

inline FVector Sample(AFortWindManager& M, const FVector& P,
    bool bHaveDeltaCylinder, const FFortWindImpulseCylinder& DeltaCylinder)
{
    FVector Wind = M.NativeWindVector_0x430;
    for (const FFortWindImpulseRadius& R : M.WindImpulses)
    {
        if (!InRadiusCaptureBounds(R.WorldBounds, P) || R.CurrentRadius <= 0.0f) continue;
        const FVector Offset = P - R.Location;
        const float Distance = Offset.Size();
        if (Distance < R.CurrentRadius)
        {
            const FVector Direction = Distance <= 1.e-8f ? FVector::ZeroVector : Offset / Distance;
            Wind += Direction * (R.CurrentMagnitude - R.CurrentMagnitude / R.CurrentRadius * Distance);
        }
    }
    for (const FFortWindImpulseCylinder& C : M.WindImpulseCylinders)
        if (!C.bIsChanging && C.Magnitude != 0.0f && InCylinderCaptureBounds(C.WorldBounds, P))
            Wind += CylinderDirectionWithFalloff(C, P) * C.Magnitude;

    if (bHaveDeltaCylinder && M.WindImpulseCylinderDeltas.Num() > 0)
    {
        const FFortWindImpulseCylinderDelta& D = M.WindImpulseCylinderDeltas[0];
        // COMISS/JB rejects unordered minimum comparisons; COMISS/JA accepts
        // unordered maximum comparisons. Preserve this asymmetry.
        const FBox& B = D.WindImpulseBounds;
        if (P.X >= B.Min.X && !(P.X > B.Max.X)
            && P.Y >= B.Min.Y && !(P.Y > B.Max.Y)
            && P.Z >= B.Min.Z && !(P.Z > B.Max.Z))
        {
            const float DX = P.X - D.DeltaCenter.X, DY = P.Y - D.DeltaCenter.Y;
            const float Distance = FMath::Sqrt(DY * DY + DX * DX);
            float Magnitude;
            // JAE selects the next test only for ordered Distance >= inner;
            // JBE selects the band for ordered <= outer or unordered.
            if (!(Distance >= D.InnerSectionRadius))
                Magnitude = D.bRippleOutward ? D.DesiredMagnitude : D.PreviousMagnitude;
            else if (!(Distance > D.OuterSectionRadius))
                Magnitude = D.SectionCurrentMagnitude;
            else
                Magnitude = D.bRippleOutward ? D.PreviousMagnitude : D.DesiredMagnitude;
            Wind += CylinderDirectionWithFalloff(DeltaCylinder, P) * Magnitude;
        }
    }
    return Wind;
}

inline FBox ResponderBounds(const FFortWindResponder& R)
{
    FBox Bounds;
    static_assert(sizeof(Bounds) == sizeof(R.NativeBounds_0x68), "Native bounds contract");
    FMemory::Memcpy(&Bounds, R.NativeBounds_0x68, sizeof(Bounds));
    return Bounds;
}

inline void UpdateMaterials(AFortWindManager& M, FFortWindResponder& R,
    float Speed, const FVector& Direction, float Time, float DeltaSeconds, FEffects& E)
{
    R.WindSpeed = Speed;
    if (R.MildWindMaterialInstances.Num() <= 0) return;
    if (!R.bHasSetupAnimatingMaterials) E.SetupAnimatingMaterials_141ddae00(M, R);
    for (FFortWindResponderMaterialVariablePairData& Pair : R.PairedVariablesData)
    {
        UCurveLinearColor* Curve = Pair.MaterialVariableIndex < 4 ? R.WindSpeedCurve : R.WindPannerSpeedCurve;
        const FLinearColor Value = Curve->GetLinearColorValue(R.WindSpeed);
        float NewSpeed = 0.0f;
        switch (Pair.MaterialVariableIndex)
        {
        case 0: case 4: NewSpeed = Value.R; break;
        case 1: case 5: NewSpeed = Value.G; break;
        case 2: case 6: NewSpeed = Value.B; break;
        case 3: case 7: NewSpeed = Value.A; break;
        }
        const float PreviousTime = (Time - DeltaSeconds) * Pair.PreviousSpeed + Pair.PreviousOffset;
        Pair.PreviousSpeed = NewSpeed;
        Pair.DeltaTimeModifiedByMaterialSpeed = NewSpeed * DeltaSeconds;
        Pair.MaterialsPreviousTime = PreviousTime;
        Pair.PreviousOffset = (NewSpeed * DeltaSeconds + PreviousTime) - NewSpeed * Time;
    }
    for (FFortWindMaterialData& Data : R.MaterialsData)
    {
        if (!Data.MID) continue;
        for (const FWindScalarMaterialInterpolationData& Scalar : Data.ScalarInterpolationData)
            E.SetScalarByIndex_14433ac70(Data.MID, Scalar.MaterialParameterIndex,
                (Scalar.LerpToValue - Scalar.LerpFromValue) * R.WindSpeed + Scalar.LerpFromValue);
        for (const FWindVectorMaterialInterpolationData& Vector : Data.VectorInterpolationData)
            E.SetVectorByIndex_14433b1f0(Data.MID, Vector.MaterialParameterIndex,
                (Vector.LerpToValue - Vector.LerpFromValue) * R.WindSpeed + Vector.LerpFromValue);
        for (const FFortWindMaterialParameterPairID& ID : Data.ParametersToSet)
        {
            // Machine code141de0e45..0e7d confirms direct compact-array indexing,
            // even though setup stores material variable IDs here. Preserve shipping behavior.
            if (!R.PairedVariablesData.IsValidIndex(ID.PairIndex)) continue;
            const auto& Pair = R.PairedVariablesData[ID.PairIndex];
            E.SetScalarByIndex_14433ac70(Data.MID, ID.SpeedParameter.VariableIndex, Pair.PreviousSpeed);
            E.SetScalarByIndex_14433ac70(Data.MID, ID.OffsetParameter.VariableIndex, Pair.PreviousOffset);
        }
        // Original skips this write at zero speed, leaving the previous vector parameter.
        if (R.WindSpeed != 0.0f)
        {
            const FBox Bounds = ResponderBounds(R);
            E.SetVectorByIndex_14433b1f0(Data.MID, Data.WindVectorParameterIndex,
                FLinearColor(Direction.X * Speed, Direction.Y * Speed, Direction.Z * Speed,
                    (Bounds.Min.Z + Bounds.Max.Z) * 0.5f));
        }
    }
}

inline void ReplaceFirst(TArray<int32>& Array, int32 From, int32 To)
{
    const int32 Index = Array.Find(From);
    if (Index != INDEX_NONE) Array[Index] = To;
}

inline void RemoveResponder(AFortWindManager& M, int32 Removed, FEffects& E)
{
    E.SpatialRemove_141dd61e0(M, Removed, ResponderBounds(M.WindResponders[Removed]));
    const int32 Last = M.WindResponders.Num() - 1;
    if (Last > 0 && Last != Removed)
    {
        const FBox LastBounds = ResponderBounds(M.WindResponders[Last]);
        E.SpatialRemove_141dd61e0(M, Last, LastBounds);
        E.SpatialInsert_141dd0bb0(M, Removed, LastBounds, nullptr);
        ReplaceFirst(M.NearbyCacheIndicesForUpdates, Last, Removed);
        ReplaceFirst(M.NearbyCacheIndicesForAudio, Last, Removed);
    }
    M.WindResponders.RemoveAtSwap(Removed, 1, true); // Nested arrays must be destroyed.
    M.NearbyCacheIndicesForUpdates.RemoveAtSwap(M.NextNearbyIndexToUpdate, 1, true);
    const int32 AudioIndex = M.NearbyCacheIndicesForAudio.Find(Removed);
    if (AudioIndex != INDEX_NONE) M.NearbyCacheIndicesForAudio.RemoveAtSwap(AudioIndex, 1, true);
    --M.NextNearbyIndexToUpdate;
}

inline float Smooth(float T) { return 3.0f * T * T - 2.0f * T * T * T; }

inline void UpdateRadiusImpulses(AFortWindManager& M, float DeltaSeconds)
{
    for (int32 Index = M.WindImpulses.Num() - 1; Index >= 0; --Index)
    {
        FFortWindImpulseRadius& R = M.WindImpulses[Index];
        const bool bRadiusChanged = R.Radius != R.CurrentRadius;
        if (!bRadiusChanged && R.Magnitude == R.CurrentMagnitude) continue;
        // Source keeps this local at desired magnitude in completion branches.
        float MagnitudeForRemoval = R.Magnitude;
        if (R.BlendTime > 0.0f)
        {
            R.CurrentBlendTime -= DeltaSeconds;
            if (R.CurrentBlendTime > 0.0f)
            {
                const float Alpha = Smooth(1.0f - R.CurrentBlendTime / R.BlendTime);
                MagnitudeForRemoval = (R.Magnitude - R.PreviousMagnitude) * Alpha + R.PreviousMagnitude;
                R.CurrentMagnitude = MagnitudeForRemoval;
                if (bRadiusChanged)
                {
                    R.CurrentRadius = (R.Radius - R.PreviousRadius) * Alpha + R.PreviousRadius;
                    const FVector Extent(R.CurrentRadius);
                    R.WorldBounds = FBox(R.Location - Extent, R.Location + Extent);
                }
            }
            else { R.CurrentMagnitude = R.Magnitude; R.CurrentRadius = R.Radius; }
        }
        else { R.CurrentMagnitude = R.Magnitude; R.CurrentRadius = R.Radius; }
        // Bounds are NOT recomputed in completion branches in this capture.
        if (R.Magnitude == 0.0f && MagnitudeForRemoval == 0.0f)
            M.WindImpulses.RemoveAtSwap(Index, 1, false);
    }
}

inline void UpdateDeltaImpulses(AFortWindManager& M, float DeltaSeconds, FEffects& E)
{
    TArray<FBox> SectionBoxes; // 16-byte local, freed on exit; contents unused by this function.
    if (M.WindImpulseCylinderDeltas.Num() > 0)
    {
        FFortWindImpulseCylinderDelta& D = M.WindImpulseCylinderDeltas[0];
        bool bSectionComplete = D.DesiredMagnitude == D.SectionCurrentMagnitude;
        if (!bSectionComplete)
        {
            if (D.SectionBlendTime > 0.0f)
            {
                D.SectionCurrentBlendTime -= DeltaSeconds;
                if (D.SectionCurrentBlendTime <= 0.0f)
                { D.SectionCurrentMagnitude = D.DesiredMagnitude; bSectionComplete = true; }
                else
                {
                    const float Target = (D.DesiredMagnitude - D.PreviousMagnitude)
                        * Smooth(1.0f - D.SectionCurrentBlendTime / D.SectionBlendTime) + D.PreviousMagnitude;
                    D.SectionCurrentMagnitude = FMath::Clamp(Target,
                        FMath::Min(D.DesiredMagnitude, D.PreviousMagnitude),
                        FMath::Max(D.DesiredMagnitude, D.PreviousMagnitude));
                    bSectionComplete = D.SectionCurrentMagnitude == D.DesiredMagnitude;
                }
            }
            else { D.SectionCurrentMagnitude = D.DesiredMagnitude; bSectionComplete = true; }
            E.BuildSectionBoxes_141dcb5d0(D, SectionBoxes);
        }
        if (bSectionComplete && !E.AdvanceDeltaSection_141dd9880(D))
        {
            const int32 UID = D.WindImpulseHandleToModify.UID;
            const float DesiredMagnitude = D.DesiredMagnitude;
            const int32 CylinderIndex = M.WindImpulseCylinders.IndexOfByPredicate(
                [UID](const FFortWindImpulseCylinder& C) { return C.Handle.UID == UID; });
            M.WindImpulseCylinderDeltas.RemoveAtSwap(0, 1, false);
            bool bSameCylinderStillFirst = false;
            if (CylinderIndex != INDEX_NONE)
            {
                FFortWindImpulseCylinder& C = M.WindImpulseCylinders[CylinderIndex];
                C.Magnitude = DesiredMagnitude;
                const int32 NextDeltaIndex = M.WindImpulseCylinderDeltas.IndexOfByPredicate(
                    [UID](const FFortWindImpulseCylinderDelta& Next) { return Next.WindImpulseHandleToModify.UID == UID; });
                bSameCylinderStillFirst = NextDeltaIndex == 0;
                C.bIsChanging = bSameCylinderStillFirst;
                if (!bSameCylinderStillFirst)
                {
                    E.CylinderChanged_142b54060(M, C);
                    if (NextDeltaIndex != INDEX_NONE) C.bIsChangePending = true;
                }
                if (C.Magnitude == 0.0f && !C.bIsChanging && !C.bIsChangePending)
                {
                    E.CylinderRemoved_142b53fa0(M, C);
                    M.WindImpulseCylinders.RemoveAtSwap(CylinderIndex, 1, false);
                }
            }
            if (!bSameCylinderStillFirst) E.StartNextDelta_141ddef30(M);
        }
    }
    // Source frees SectionBoxes before the final cylinder cleanup; release at same boundary.
    SectionBoxes.Empty();
    for (int32 Index = M.WindImpulseCylinders.Num() - 1; Index >= 0; --Index)
    {
        const auto& C = M.WindImpulseCylinders[Index];
        if (C.Magnitude == 0.0f && !C.bIsChanging && !C.bIsChangePending)
        {
            E.CylinderRemoved_142b53fa0(M, C);
            M.WindImpulseCylinders.RemoveAtSwap(Index, 1, false);
        }
    }
}

inline void SampleWind_141ddfad0(AFortWindManager& M, FState State, float DeltaSeconds, FEffects& E)
{
    if (!E.WindEnabled_14812a5f8()) return;
    if (E.IsDedicatedServer_143f7a920()) { E.LogAndDestroyDedicated_143f6c200(M); return; }
    float WorldTime;
    if (!E.GetWorldTimeOrLogNull(M, WorldTime)) return;
    E.BeginStat(TEXT("STAT_Fort_UpdateWindResponders"));
    // Even with no deltas this temporary consumes exactly one shared native handle UID.
    FFortWindImpulseCylinder DeltaCylinder{};
    bool bHaveDeltaCylinder = false;
    if (M.WindImpulseCylinderDeltas.Num() > 0)
    {
        bHaveDeltaCylinder = E.GetCylinder_141dcf3f0(M,
            M.WindImpulseCylinderDeltas[0].WindImpulseHandleToModify, DeltaCylinder);
    }
    const int32 Count = M.bAllowWindImpulses
        ? FMath::Min(M.NearbyCacheIndicesForUpdates.Num(), M.NativeWindUpdateBudget_0x220)
        : M.NearbyCacheIndicesForUpdates.Num();
    for (int32 Iteration = 0; Iteration < Count; ++Iteration)
    {
        const int32 Cursor = M.NextNearbyIndexToUpdate;
        const int32 ResponderIndex = M.NearbyCacheIndicesForUpdates[Cursor];
        // Capture tests Cursor against responder count; do not silently substitute cached index.
        if (Cursor >= 0 && Cursor < M.WindResponders.Num())
        {
            FFortWindResponder& R = M.WindResponders[ResponderIndex];
            if (R.WindUpdatingBuildingSMActor && !E.ActorHasNativeInvalidFlag(R.WindUpdatingBuildingSMActor))
            {
                const FVector Wind = M.bAllowWindImpulses ? Sample(M,
                    E.ActorRootLocationOrZero(R.WindUpdatingBuildingSMActor), bHaveDeltaCylinder, DeltaCylinder)
                    : M.NativeWindVector_0x430;
                const float Length = Wind.Size();
                const FVector Direction = Length <= 1.e-8f ? FVector::ZeroVector : Wind / Length;
                const float Speed = FMath::Clamp(Length, 0.0f, 1.0f);
                // Direction-only changes do not cause a material write.
                if (FMath::Abs(Speed - R.WindSpeed) > 1.e-8f)
                    UpdateMaterials(M, R, Speed, Direction, WorldTime, DeltaSeconds, E);
            }
            else RemoveResponder(M, ResponderIndex, E);
        }
        M.NextNearbyIndexToUpdate = M.NearbyCacheIndicesForUpdates.Num() < 1 ? 0
            : (M.NextNearbyIndexToUpdate + 1) % M.NearbyCacheIndicesForUpdates.Num();
    }
    E.EndStat();
    if (!M.bAllowWindImpulses) return;
    E.BeginStat(TEXT("STAT_Fort_UpdateWindImpulses"));
    UpdateRadiusImpulses(M, DeltaSeconds);
    E.EndStat();
    UpdateDeltaImpulses(M, DeltaSeconds, E);
}
} // namespace FortWindSamplingDraft
