// CONFIDENCE: HIGH. Complete original 0x14233e6e0; build/runtime pending.
UWorld* World = GetWorld();
AFortGameStateAthena* AthenaState = World ? World->GetGameState<AFortGameStateAthena>() : nullptr;
if (UFortControllerComponent_SkydiveFeedback* Feedback = NativeControllerFeedback1423240e0())
{
    Feedback->NativeSetFeedback1418bfe90(1);
}
if (AthenaState && AthenaState->NativeConsumeParachuteOpen14199ce60(this, bFromInput)) return;
if ((!bFromInput || GetLocalRole() == ROLE_Authority) && NativeCanDeployParachute14230bcb0())
{
    bool bConsumeEvent = false;
    OnOpenParachute(bConsumeEvent);
    if (bConsumeEvent) return;
    GetCharacterMovement()->SetMovementMode(MOVE_Custom, 3);
}
if (AFortPlayerController* FortController = Cast<AFortPlayerController>(Controller))
{
    if (FortController->IsLocalController()) FortController->NativeControllerVirtual1d10();
}
const auto& Tags = FFortNativeGameplayTagsRuntime::Get();
const FGameplayTag& Action = Tags.GetTagAtOriginalOffset(0x6a0);
if (!HasNativeAction(Action))
{
    NativeStartAction142353060(Action);
    if (AbilitySystemComponent)
    {
        FGameplayEventData Payload;
        Payload.Instigator = this;
        Payload.Target = this;
        GetOwnedGameplayTags(Payload.InstigatorTags);
        GetOwnedGameplayTags(Payload.TargetTags);
        AbilitySystemComponent->HandleGameplayEvent(Tags.GetTagAtOriginalOffset(0x198), &Payload);
        AbilitySystemComponent->UpdateTagMap(Tags.GetTagAtOriginalOffset(0x1f70), 1);
    }
}
