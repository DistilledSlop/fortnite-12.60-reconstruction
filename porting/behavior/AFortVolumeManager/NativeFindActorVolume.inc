// CONFIDENCE: high
// 0x141c7c5c0, with original actor-bounds and IFortVolumeAccessor paths.
QUICK_SCOPE_CYCLE_COUNTER(STAT_Find_Volume);
if (!Actor || VolumeObjects.Num() == 0)
{
    return nullptr;
}
FBox ActorBox;
ABuildingSMActor* Building = Cast<ABuildingSMActor>(Actor);
UStaticMeshComponent* BuildingMesh = Building ? Building->GetBuildingMeshComponent() : nullptr;
if (BuildingMesh)
{
    const FBoxSphereBounds& MeshBounds = BuildingMesh->Bounds;
    ActorBox = FBox(MeshBounds.Origin - MeshBounds.BoxExtent, MeshBounds.Origin + MeshBounds.BoxExtent);
}
else
{
    const bool bNonColliding = !Actor->IsA<APawn>() && !Actor->IsA<AInfo>();
    ActorBox = Actor->GetComponentsBoundingBox(bNonColliding, false);
}
AFortVolume* Selected = nullptr;
for (AFortVolume* Candidate : VolumeObjects)
{
    if (!Candidate || Candidate == Actor)
    {
        continue;
    }
    const bool bMember = Candidate->NativeActorMembership.Contains(TWeakObjectPtr<AActor>(Actor));
    bool bContained = bMember;
    if (!Actor->IsA<AInfo>() && !bMember)
    {
        bContained = Candidate->NativeContainsActorBounds(Actor, FBoxSphereBounds(ActorBox), true);
    }
    if (!bContained)
    {
        continue;
    }
    if (!Selected)
    {
        Selected = Candidate;
    }
    else
    {
        FBoxSphereBounds SelectedBounds;
        FBoxSphereBounds CandidateBounds;
        Selected->NativeGetLocationBounds(SelectedBounds);
        Candidate->NativeGetLocationBounds(CandidateBounds);
        if (CandidateBounds.BoxExtent.X < SelectedBounds.BoxExtent.X &&
            CandidateBounds.BoxExtent.Y < SelectedBounds.BoxExtent.Y)
        {
            Selected = Candidate;
        }
    }
}
if (!Selected && Actor->IsA<AInfo>())
{
    // The original reentry guard is process-wide, shared by all managers.
    static bool bResolvingVolumeAccessor = false;
    if (!bResolvingVolumeAccessor)
    {
        if (IFortVolumeAccessor* Accessor = Cast<IFortVolumeAccessor>(Actor))
        {
            bResolvingVolumeAccessor = true;
            Selected = Accessor->NativeGetVolume();
            bResolvingVolumeAccessor = false;
        }
    }
}
return Selected;
