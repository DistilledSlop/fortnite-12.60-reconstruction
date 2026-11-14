// CONFIDENCE: HIGH. Original1426b3730; gates run even for a false request.
AFortPlayerController* TargetController = Cast<AFortPlayerController>(NativeResolveController1426a07c0());
bool bEffectiveTargeting = bNewTargeting;
if (!NativeCanTarget14268efc0())
{
    bEffectiveTargeting = false;
}
else
{
    bool bAllowed = true;
    if (TargetController)
    {
        bAllowed = TargetController->NativeCanPerformAction15f0(
            FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x768));
    }
    if (bAllowed && NativeIsReloadAbilityActive830())
    {
        bAllowed = WeaponData && WeaponData->NativeAllowTargetingDuringReload142684ac0();
    }
    if (bAllowed && !IsEquipped() && !bIsEquippingWeapon
        && NativeBlockTargetingWhileUnequipped1478df2c4() != 0)
    {
        bAllowed = false;
    }
    if (!bAllowed)
    {
        bEffectiveTargeting = false;
    }
}
if (bIsTargeting != bEffectiveTargeting)
{
    bIsTargeting = bEffectiveTargeting;
    LastTargetingTransitionTime = GetWorld()->GetTimeSeconds();
    OnSetTargeting(bIsTargeting);
    OnLocalTargetingChanged.Broadcast(bIsTargeting);
    if (GetLocalRole() < ROLE_Authority && TargetController)
    {
        if (bIsTargeting && TargetingStartSound)
        {
            if (!TargetingAudioChannels[0])
            {
                TargetingAudioChannels[0] = NewObject<UAudioComponent>(this, FName(TEXT("TargetingStart")), RF_Transient);
                NativeConfigureAudio1426a2720(TargetingAudioChannels[0]);
            }
            TargetingAudioChannels[0]->SetSound(TargetingStartSound);
            TargetingAudioChannels[0]->Play(0.0f);
        }
        else if (TargetingEndSound)
        {
            if (!TargetingAudioChannels[1])
            {
                TargetingAudioChannels[1] = NewObject<UAudioComponent>(this, FName(TEXT("TargetingEnd")), RF_Transient);
                NativeConfigureAudio1426a2720(TargetingAudioChannels[1]);
            }
            TargetingAudioChannels[1]->SetSound(TargetingEndSound);
            TargetingAudioChannels[1]->Play(0.0f);
        }
    }
}
// This callback always receives the effective argument, including after reentrant notifications.
if (AFortPlayerPawn* InstigatingPawn = Cast<AFortPlayerPawn>(GetInstigator()))
{
    InstigatingPawn->NativeOnWeaponTargeting1423336c0(bEffectiveTargeting);
}
