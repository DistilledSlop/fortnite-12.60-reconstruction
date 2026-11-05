#pragma once

#include "FFortWindImpulseCylinderDelta.h"

namespace MapsWindDeltaRuntimeDraft
{
// 0x142c59040: preserve ordered comparisons and selection on equality.
inline FBox Overlap_142c59040(const FBox& A, const FBox& B)
{
    if ((A.Min.X < B.Max.X || A.Min.X == B.Max.X) && B.Min.X <= A.Max.X &&
        (A.Min.Y < B.Max.Y || A.Min.Y == B.Max.Y) && B.Min.Y <= A.Max.Y &&
        (A.Min.Z < B.Max.Z || A.Min.Z == B.Max.Z) && B.Min.Z <= A.Max.Z)
    {
        const FVector Min(B.Min.X <= A.Min.X ? A.Min.X : B.Min.X,
                          B.Min.Y <= A.Min.Y ? A.Min.Y : B.Min.Y,
                          B.Min.Z <= A.Min.Z ? A.Min.Z : B.Min.Z);
        const FVector Max(B.Max.X <= A.Max.X ? B.Max.X : A.Max.X,
                          B.Max.Y <= A.Max.Y ? B.Max.Y : A.Max.Y,
                          B.Max.Z <= A.Max.Z ? B.Max.Z : A.Max.Z);
        return FBox(Min, Max);
    }
    return FBox(ForceInit);
}

inline bool AdvanceDeltaSection_141dd9880(FFortWindImpulseCylinderDelta& D)
{
    D.SectionCurrentBlendTime = D.SectionBlendTime;
    D.SectionCurrentMagnitude = D.PreviousMagnitude;

    float OuterRadius;
    if (!D.bRippleOutward)
    {
        OuterRadius = D.InnerSectionRadius;
        D.OuterSectionRadius = OuterRadius;
        if (OuterRadius == 0.0f)
            return false;
        D.InnerSectionRadius = OuterRadius - D.SectionWidth;
        if (D.InnerSectionRadius < 0.0f)
            D.InnerSectionRadius = 0.0f;
    }
    else
    {
        OuterRadius = D.OuterSectionRadius;
        const float MaximumRadius = D.MaximumRadius;
        D.InnerSectionRadius = OuterRadius;
        if (OuterRadius == MaximumRadius)
            return false;
        OuterRadius = OuterRadius + D.SectionWidth;
        D.OuterSectionRadius = OuterRadius;
        if (MaximumRadius < OuterRadius)
        {
            D.OuterSectionRadius = MaximumRadius;
            OuterRadius = MaximumRadius;
        }
    }

    FBox OuterBox;
    OuterBox.Min = FVector(D.DeltaCenter.X - OuterRadius,
                           D.DeltaCenter.Y - OuterRadius,
                           D.WindImpulseBounds.Min.Z);
    OuterBox.Max = FVector(D.DeltaCenter.X + OuterRadius,
                           D.DeltaCenter.Y + OuterRadius,
                           D.WindImpulseBounds.Max.Z);
    OuterBox.IsValid = 1;
    D.OuterWorldBounds = OuterBox;
    D.OuterWorldBounds = Overlap_142c59040(D.OuterWorldBounds, D.WindImpulseBounds);

    const float InnerBoxRadius = D.InnerSectionRadius * 0.70710677f;
    FBox InnerBox;
    InnerBox.Min = FVector(D.DeltaCenter.X - InnerBoxRadius,
                           D.DeltaCenter.Y - InnerBoxRadius,
                           D.WindImpulseBounds.Min.Z);
    InnerBox.Max = FVector(D.DeltaCenter.X + InnerBoxRadius,
                           D.DeltaCenter.Y + InnerBoxRadius,
                           D.WindImpulseBounds.Max.Z);
    InnerBox.IsValid = 1;
    D.InnerWorldBounds = InnerBox;
    D.InnerWorldBounds = Overlap_142c59040(D.InnerWorldBounds, D.WindImpulseBounds);
    return true;
}
}
