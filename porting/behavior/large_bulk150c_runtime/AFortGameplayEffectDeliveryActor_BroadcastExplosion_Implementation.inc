// Native vtable +0x698, 0x1426a4970. CONFIDENCE: high
if (!bNativeExplosionBroadcastBlocked)
{
    bHasExploded = true;
    if (!bCanExplodeMultipleTimes)
    {
        bNativeExplosionBroadcastBlocked = true;
    }
    struct FExplosionParameters
    {
        TArray<AActor*> Actors;
        TArray<FHitResult> Results;
    };
    FExplosionParameters Parameters = {HitActors, HitResults};
    ProcessEvent(FindFunctionChecked(TEXT("OnExploded")), &Parameters);
    NativeExploded.Broadcast(this, HitActors, HitResults);
    NativeActorOverlapCounts.Empty(0);
}
