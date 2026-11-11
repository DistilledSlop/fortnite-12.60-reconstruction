// CONFIDENCE: MEDIUM. One-shot teardown of a remote-controlled pawn, run on the authority. Fires the destroy event, queues the deferred destroy callback and a 1 second controller-detach timer, then finishes teardown (life span, visibility, component hit/overlap unbinding). The tail runs whenever the trigger flag was still clear, even if the timer block was skipped.
// Original addresses: exec 0x142ac2b80 (thunk; body inline in the same function). Callees: 0x142ac1190 (destroy event via ProcessEvent), 0x14243bb80 (deferred destroy callback, deps), 0x14243a470 (DetachRemoteControlledPawn), 0x144627d90 (SetTimerForNextTick), 0x1446278d0 (SetTimer), 0x143f83090 and 0x143f84060 (visibility helpers), 0x142ed32e0 (dynamic unbind).

if (Role == ROLE_Authority && !bDestroyTriggered)
{
    ForceNetUpdate();
    NativeFireDestroyRemoteControlPawnEvent();

    UWorld* World = GetWorld();
    if (World != nullptr)
    {
        // Timer manager of the world; the decompile also has a GameInstance-owned fallback (LOW).
        FTimerManager& TimerManager = World->GetTimerManager();

        FTimerDelegate DeferredDestroyDelegate = FTimerDelegate::CreateUObject(this, &AFortRemoteControlledPawnAthena::DeferredDestroyCallback);
        TimerManager.SetTimerForNextTick(DeferredDestroyDelegate);

        FTimerDelegate DetachDelegate = FTimerDelegate::CreateUObject(this, &AFortRemoteControlledPawnAthena::DetachRemoteControlledPawn);
        FTimerHandle DetachTimerHandle;
        TimerManager.SetTimer(DetachTimerHandle, DetachDelegate, 1.0f, false, -1.0f);
    }
}

if (!bDestroyTriggered)
{
    bDestroyTriggered = true;

    if (Role == ROLE_Authority)
    {
        SetLifeSpan(2.0f);
    }

    NativeSetActorComponentsHidden(false);
    NativeSetAttachedVisibility(false);

    UCapsuleComponent* Capsule = CapsuleComponent;
    if (Capsule != nullptr)
    {
        static const FName OnComponentHitName(TEXT("OnComponentHit"));
        static const FName OnComponentBeginOverlapName(TEXT("OnComponentBeginOverlap"));

        if (Capsule->bBoundOnComponentHit)
        {
            Capsule->bBoundOnComponentHit = NativeRemoveDynamicComponentBinding(Capsule, OnComponentHitName, this);
        }
        if (Capsule->bBoundOnComponentBeginOverlap)
        {
            Capsule->bBoundOnComponentBeginOverlap = NativeRemoveDynamicComponentBinding(Capsule, OnComponentBeginOverlapName, this);
        }
    }
}
