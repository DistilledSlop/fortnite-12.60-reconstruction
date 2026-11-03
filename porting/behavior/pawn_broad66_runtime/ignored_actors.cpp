// Rough lift of 0x142a7e4e0, omitting only exec frame bookkeeping.
// Opaque operations retain their native addresses until engine APIs are pinned.
void AFortPlayerPawn::RemoveExpiredIgnoredActorsAndCheckForNextTimer()
{
    UWorld* World = GetWorld();
    if (!World) return;
    float Next = FLT_MAX;
    for (int32 I = IgnoreActors.Num() - 1; I >= 0; --I) {
        const FIgnoreCollisionActor& Entry = IgnoreActors[I];
        const float Elapsed = World->TimeSeconds - Entry.TimeIgnoreStarted;
        if (Elapsed < Entry.IgnoreDuration) {
            Next = FMath::Min(Next, FMath::Max(Entry.IgnoreDuration - Elapsed, 0.01f));
        } else {
            if (Entry.IgnoreActor)
                Native_1440f4040(/* pawn component at +0x288 */ GetCapsuleComponent(), Entry.IgnoreActor, false);
            // Native uses ordered removal and shrinks the array, not RemoveAtSwap.
            IgnoreActors.RemoveAt(I);
        }
    }
    auto& TimerManager = Native_WorldTimerManager(World); // world+0x188 ? +0xc0 : world+0x320
    if (FMath::Abs(Next - FLT_MAX) <= 1.e-8f) {
        Native_ClearTimerIfPresent_144621fb0_144627760(TimerManager, NativeTimerHandle_0x1c80);
        NativeTimerHandle_0x1c80 = {};
    } else {
        // One-shot weak-object delegate bound to native callback 0x142347f20.
        Native_SetTimer_1446278d0(TimerManager, NativeTimerHandle_0x1c80, this,
                                  NativeCallback_142347f20, Next);
    }
}
