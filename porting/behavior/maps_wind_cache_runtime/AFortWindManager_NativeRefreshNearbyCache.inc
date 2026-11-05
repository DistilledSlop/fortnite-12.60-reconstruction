// CONFIDENCE: HIGH (source recovery; build/runtime validation pending).
// Native 0x141ddf5c0.
// Signature: void AFortWindManager::NativeRefreshNearbyCache(const FVector& ViewerPosition)
// Requires UnrealEngine.h and GenericQuadTree.h. RandHelper uses the engine platform
// RNG; Windows matches the recovered CRT range and arithmetic, while other platforms
// may produce a different random stream.
if (NearbyCacheIndicesForAudio.Num() > 0 || NearbyCacheIndicesForUpdates.Num() > 0)
{
    const FVector Movement = NativeNearbyCachePosition_0x30c - ViewerPosition;
    const float MovementSquared = Movement.X * Movement.X + Movement.Y * Movement.Y + Movement.Z * Movement.Z;
    if (MovementSquared < ViewerMovementDistanceForRefresh * ViewerMovementDistanceForRefresh)
        return;
    NearbyCacheIndicesForAudio.Reset();
    NearbyCacheIndicesForUpdates.Reset();
}

if (!NativeWindResponderTree_0x2e0)
    return;

NativeNearbyCachePosition_0x30c = ViewerPosition;
// One function-static array shared by all managers and both queries, retaining capacity.
static TArray<int32> ScratchIndices;
const auto Query = [this](float Radius)
{
    ScratchIndices.Reset();
    const FVector& Position = NativeNearbyCachePosition_0x30c;
    NativeWindResponderTree_0x2e0->GetElements(
        FBox2D(FVector2D(Position.X - Radius, Position.Y - Radius),
               FVector2D(Position.X + Radius, Position.Y + Radius)), ScratchIndices);
};
const auto DistanceSquared = [this](const FFortWindResponder& Responder)
{
    FBox Bounds;
    static_assert(sizeof(Bounds) == sizeof(Responder.NativeBounds_0x68), "native wind bounds layout changed");
    FMemory::Memcpy(&Bounds, Responder.NativeBounds_0x68, sizeof(Bounds));
    const float X = (Bounds.Max.X + Bounds.Min.X) * 0.5f - NativeNearbyCachePosition_0x30c.X;
    const float Y = (Bounds.Max.Y + Bounds.Min.Y) * 0.5f - NativeNearbyCachePosition_0x30c.Y;
    const float Z = (Bounds.Max.Z + Bounds.Min.Z) * 0.5f - NativeNearbyCachePosition_0x30c.Z;
    return X * X + Y * Y + Z * Z;
};

Query(UpdateWindMaxResponderDistance);
const float ViewDistanceScale = GetCachedScalabilityCVars().ViewDistanceScale;
for (const int32 Index : ScratchIndices)
{
    const FFortWindResponder& Responder = WindResponders[Index];
    if (Responder.WindUpdatingBuildingSMActor)
    {
        const float ScaledDistance = ViewDistanceScale * Responder.NativeEffectiveDistance_0x88;
        const float Radius = (UpdateWindMaxResponderDistance <= ScaledDistance
            ? UpdateWindMaxResponderDistance : ScaledDistance) + ViewerMovementDistanceForRefresh;
        if (DistanceSquared(Responder) < Radius * Radius)
            NearbyCacheIndicesForUpdates.Add(Index);
    }
}
const int32 UpdateCount = NearbyCacheIndicesForUpdates.Num();
if (UpdateCount > 0)
{
    NextNearbyIndexToUpdate = FMath::RandHelper(UpdateCount);
}

if (bAllowResponderAudio)
{
    Query(AudioWindMaxResponderDistance);
    const float Radius = ViewerMovementDistanceForRefresh + AudioWindMaxResponderDistance;
    for (const int32 Index : ScratchIndices)
    {
        const FFortWindResponder& Responder = WindResponders[Index];
        if (Responder.WindUpdatingBuildingSMActor && DistanceSquared(Responder) < Radius * Radius)
            NearbyCacheIndicesForAudio.Add(Index);
    }
}
