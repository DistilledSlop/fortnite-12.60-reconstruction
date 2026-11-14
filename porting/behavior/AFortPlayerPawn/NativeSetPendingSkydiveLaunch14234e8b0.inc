// CONFIDENCE: HIGH. Original14234e8b0. Payload destruction precedes from-bus state change.
bPendingSkydiveLaunch = bPending;
if (!bPending)
{
    return;
}

if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
{
    // The original destroys the payload before clearing bIsSkydivingFromBus.
    FGameplayEventData Payload;
    Payload.Instigator = this;
    Payload.Target = this;
    const FGameplayTag EventTag = FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(
        bIsSkydivingFromBus ? 0x1a8 : 0x1a0);
    ASC->HandleGameplayEvent(EventTag, &Payload);
}

bIsSkydivingFromBus = false;
if (AFortPlayerStateZone* FortState = Cast<AFortPlayerStateZone>(PlayerState))
{
    FortState->NativeSetHasEverSkydivedFromBus1424293c0(true);
    FortState->NativeSetHasEverSkydivedFromBusAndLanded1424293d0(true);
}
if (Controller && NativeNetMode143f7a920() != NM_DedicatedServer)
{
    // Reload Controller after net-mode resolution, matching the original.
    UActorComponent* Component = Controller->FindComponentByClass(
        UFortControllerComponent_SkydiveFeedback::StaticClass());
    if (Component)
    {
        static_cast<UFortControllerComponent_SkydiveFeedback*>(Component)->NativeSetFeedback1418bfe90(3);
    }
}
