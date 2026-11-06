// CONFIDENCE: HIGH; see HANDOFF.md.
float ChargeRate = 1.0f;
if (AFortPawn* Pawn = Cast<AFortPawn>(GetInstigator()))
{
    if (UAbilitySystemComponent* AbilityComp = Pawn->GetAbilitySystemComponent())
    {
        static FProperty* ChargeRateProperty = FindFieldChecked<FProperty>(
            UFortWeaponAttrSet::StaticClass(), FName(TEXT("WeaponChargeRate")));
        static const FGameplayEffectAttributeCaptureDefinition CaptureDefinition(
            FGameplayAttribute(ChargeRateProperty), EGameplayEffectAttributeCaptureSource::Source, true);
        FGameplayEffectAttributeCaptureSpec CaptureSpec(CaptureDefinition);
        AbilityComp->CaptureAttributeForGameplayEffect(CaptureSpec);
        FGameplayTagContainer WeaponTags;
        if (WeaponData)
        {
            WeaponData->GetOwnedGameplayTags(WeaponTags);
        }
        FGameplayTagContainer SourceTags;
        AbilityComp->GetOwnedGameplayTags(SourceTags);
        SourceTags.AppendTags(WeaponTags);
        FAggregatorEvaluateParameters Parameters;
        Parameters.SourceTags = &SourceTags;
        CaptureSpec.AttemptCalculateAttributeMagnitudeWithBase(Parameters, 1.0f, ChargeRate);
        if (!(ChargeRate > 0.05f))
        {
            ChargeRate = 0.05f;
        }
    }
}
OutMin = 0.0f;
OutMax = 0.0f;
if (const FFortBaseWeaponStats* Stats = NativeGetWeaponStats())
{
    OutMin = Stats->MinChargeTime / ChargeRate;
    OutMax = Stats->MaxChargeTime / ChargeRate;
}
