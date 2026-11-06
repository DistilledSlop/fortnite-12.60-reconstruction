// UNCOUNTED DRAFT: not compiled or included in the behavior manifest.
// Lift of 0x141c6a510 and 0x141c7c5c0. The membership lifecycle must be
// restored before these readers can be integrated. No empty membership cache.
#include "AFortVolume.h"
#include "AFortVolumeManager.h"
#include "ABuildingSMActor.h"
#include "UFortVolumeAccessor.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Info.h"
#include "GameFramework/Pawn.h"

namespace FortVolumeGeometryDraft
{
// Unresolved adapter to the real weak actor set at original AFortVolume+0x318.
// Requires the AddActor/RemoveActor/EndPlay/clear lifecycle, not a new empty set.
extern bool UnresolvedMembershipContains(const AFortVolume&, const AActor*);
// Process-global original byte 0x14812316c; storage and all references need pinning.
extern bool UnresolvedAccessorRecursionGuard;

bool IsActorWithinVolume141c6a510(
    const AFortVolume& Volume, const AActor* Actor,
    const FBoxSphereBounds& ActorBounds, bool bUseBounds)
{
    QUICK_SCOPE_CYCLE_COUNTER(STAT_Is_Actor_Within_Volume);
    if (!Actor)
    {
        return false;
    }
    const float X = ActorBounds.BoxExtent.X;
    const float Y = ActorBounds.BoxExtent.Y;
    const float Z = ActorBounds.BoxExtent.Z;
    const bool bZeroBounds = !(1.0e-8f < FMath::Sqrt(X * X + Y * Y + Z * Z)) &&
                             !(1.0e-8f < FMath::Abs(ActorBounds.SphereRadius));
    if (UnresolvedMembershipContains(Volume, Actor))
    {
        return true;
    }
    if (bZeroBounds || !bUseBounds)
    {
        return !Actor->IsA<AInfo>() && Volume.NativeContainsLocation(Actor->GetActorLocation());
    }
    if (Volume.BoundsShape == EVolumeShape::Box)
    {
        const FBox Box = Volume.NativeGetBoundsBox();
        const FVector Low = ActorBounds.Origin - ActorBounds.BoxExtent;
        const FVector High = ActorBounds.Origin + ActorBounds.BoxExtent;
        // Native code rejects strict out-of-bounds comparisons. Do not rewrite
        // these as >= conjunctions: unordered comparisons have different results.
        const bool bLowInside = !(Low.X < Box.Min.X - 175.0f || Box.Max.X + 175.0f < Low.X ||
                                  Low.Y < Box.Min.Y - 175.0f || Box.Max.Y + 175.0f < Low.Y ||
                                  Low.Z < Box.Min.Z - 175.0f || Box.Max.Z + 175.0f < Low.Z);
        const bool bHighInside = !(High.X < Box.Min.X - 175.0f || Box.Max.X + 175.0f < High.X ||
                                   High.Y < Box.Min.Y - 175.0f || Box.Max.Y + 175.0f < High.Y ||
                                   High.Z < Box.Min.Z - 175.0f || Box.Max.Z + 175.0f < High.Z);
        return bLowInside && bHighInside;
    }
    if (Volume.BoundsShape == EVolumeShape::Sphere)
    {
        FBoxSphereBounds Bounds;
        Volume.NativeGetLocationBounds(Bounds);
        const float Radius = Bounds.SphereRadius + 0.0001f;
        if (ActorBounds.SphereRadius <= Radius)
        {
            const float DX = ActorBounds.Origin.X - Bounds.Origin.X;
            const float DY = ActorBounds.Origin.Y - Bounds.Origin.Y;
            const float DZ = ActorBounds.Origin.Z - Bounds.Origin.Z;
            const float Difference = Radius - ActorBounds.SphereRadius;
            return DX * DX + DY * DY + DZ * DZ <= Difference * Difference;
        }
    }
    return false;
}

AFortVolume* FindActorVolume141c7c5c0(const AFortVolumeManager& Manager, AActor* Actor)
{
    QUICK_SCOPE_CYCLE_COUNTER(STAT_Find_Volume);
    if (!Actor || Manager.VolumeObjects.Num() == 0)
    {
        return nullptr;
    }
    FBox ActorBox;
    if (const ABuildingSMActor* Building = Cast<ABuildingSMActor>(Actor))
    {
        if (const UStaticMeshComponent* Mesh = Building->StaticMeshComponent)
        {
            ActorBox = Mesh->Bounds.GetBox();
        }
        else
        {
            ActorBox = Actor->GetComponentsBoundingBox(false, false);
        }
    }
    else
    {
        const bool bNonColliding = !Actor->IsA<APawn>() && !Actor->IsA<AInfo>();
        ActorBox = Actor->GetComponentsBoundingBox(bNonColliding, false);
    }
    AFortVolume* Selected = nullptr;
    // The native loop snapshots the array end before calling geometry helpers.
    AFortVolume* const* CandidateIt = Manager.VolumeObjects.GetData();
    AFortVolume* const* const CandidateEnd = CandidateIt + Manager.VolumeObjects.Num();
    for (; CandidateIt != CandidateEnd; ++CandidateIt)
    {
        AFortVolume* Candidate = *CandidateIt;
        if (!Candidate || Candidate == Actor)
        {
            continue;
        }
        bool bEligible = UnresolvedMembershipContains(*Candidate, Actor);
        if (!Actor->IsA<AInfo>() && !bEligible)
        {
            const FVector Size = ActorBox.Max - ActorBox.Min;
            FBoxSphereBounds Bounds;
            Bounds.BoxExtent = Size * 0.5f;
            Bounds.Origin = ActorBox.Min + Bounds.BoxExtent;
            Bounds.SphereRadius = FMath::Sqrt(
                (Size.X * Size.X + Size.Y * Size.Y + Size.Z * Size.Z) * 0.25f);
            bEligible = IsActorWithinVolume141c6a510(*Candidate, Actor, Bounds, true);
        }
        if (!bEligible)
        {
            continue;
        }
        if (!Selected)
        {
            Selected = Candidate;
            continue;
        }
        // Both calls share one native output scratch area; the helper can leave
        // it untouched when there is no bounds component. Preserve reuse/order.
        FBoxSphereBounds Scratch;
        Selected->NativeGetLocationBounds(Scratch);
        const float SelectedX = Scratch.BoxExtent.X;
        const float SelectedY = Scratch.BoxExtent.Y;
        Candidate->NativeGetLocationBounds(Scratch);
        if (!(Scratch.BoxExtent.X >= SelectedX) && SelectedY > Scratch.BoxExtent.Y)
        {
            Selected = Candidate;
        }
    }
    if (!Selected && Actor->IsA<AInfo>() && !UnresolvedAccessorRecursionGuard)
    {
        IFortVolumeAccessor* Accessor = static_cast<IFortVolumeAccessor*>(
            Actor->GetNativeInterfaceAddress(UFortVolumeAccessor::StaticClass()));
        if (Accessor)
        {
            UnresolvedAccessorRecursionGuard = true;
            Selected = Accessor->NativeGetVolume();
            UnresolvedAccessorRecursionGuard = false;
        }
    }
    return Selected;
}
}
