// CONFIDENCE: HIGH. Original0x1426b7a40; dynamic weapon +0x848.
FFortWeaponAutoReloadHitchScope HitchScope;
if (!IsEquipped()) return;
if (AFortPawn* Pawn = Cast<AFortPawn>(GetInstigator()))
{
    if (UAbilitySystemComponent* ASC = Pawn->GetAbilitySystemComponent())
    {
        FGameplayTagContainer InteractionTags;
        InteractionTags.AddTag(FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x748));
        InteractionTags.AddTag(FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x6f8));
        if (ASC->HasAnyMatchingGameplayTags(InteractionTags)) return;
    }
    // Original14268c4c0 re-reads instigator after the ASC interface call.
    if (AFortPlayerPawn* PlayerPawn = Cast<AFortPlayerPawn>(GetInstigator()))
    {
        const float EmoteStart = PlayerPawn->EmoteStartTime;
        if (GetWorld()->TimeSeconds - EmoteStart < 0.4000999927520752f
            || PlayerPawn->bIsPlayingEmote) return;
    }
}
if (AFortPlayerController* PC = Cast<AFortPlayerController>(NativeResolveController1426a07c0()))
{
    if (PC->IsLocalPlayerController() && PC->bWantsToSprint
        && PC->NativeForwardInput14238f110() && PC->bSprintCancelsReload) return;
}
// Capture definition before the reentrant virtual predicate; targeting is read afterwards.
UFortWeaponItemDefinition* Definition = Cast<UFortWeaponItemDefinition>(WeaponData);
if (NativeCanReload828(false, nullptr) && Definition
    && (!bIsTargeting || !Definition->bTargetingPreventsReload))
{
    NativeReload838();
}
