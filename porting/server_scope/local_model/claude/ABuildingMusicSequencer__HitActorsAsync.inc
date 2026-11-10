// CONFIDENCE: MEDIUM. Turns the sequencer volume into a world-space oriented box (origin transformed, extent scaled, rotation kept) and starts an async overlap; its handle is stored for the callback.
// Original address: 0x14274b1b0 (exec). Native body: 0x141c0cfa0.
SCOPE_CYCLE_COUNTER(STAT_MusicSequencerHitActors);
UWorld* World = GetWorld();
if (World != nullptr)
{
    const FVector BoxCenter = Transform.TransformPosition(Bounds.Origin);
    const FVector BoxExtent = Transform.GetScale3D() * Bounds.BoxExtent;
    const FQuat BoxRotation = Transform.GetRotation();

    FCollisionQueryParams QueryParams(FName(TEXT("MusicSequencerHitActors")));
    FCollisionObjectQueryParams ObjectParams(MusicSequencerHitChannels);
    HitActorsCallback = [this](FHitActorsHandle Handle, const FOverlapResults& Results)   // native 0x141c13280
    {
        OnHitActorsResult(Handle, Results);
    };
    HitActorsQueryHandle = World->AsyncOverlapByObjectType(BoxCenter, BoxRotation, BoxExtent, QueryParams, ObjectParams, HitActorsCallback);   // native 0x14409cc80
}
