// CONFIDENCE: HIGH
// Recovered item stack/ownership behavior; see HANDOFF.md.
int32 StackLimit = MaxStackSize.AsInteger(0.0f, nullptr);
if (static_cast<uint32>(StackLimit) > 0x7ffffffeu)
{
    return MAX_int32;
}
if (AbilityComp)
{
    static FProperty* ItemMaxStackProperty = FindFieldChecked<FProperty>(
        UFortPlayerAttrSet::StaticClass(), FName(TEXT("ItemMaxStack")));
    static const FGameplayEffectAttributeCaptureDefinition CaptureDefinition(
        FGameplayAttribute(ItemMaxStackProperty), EGameplayEffectAttributeCaptureSource::Source, true);
    FGameplayEffectAttributeCaptureSpec CaptureSpec(CaptureDefinition);
    AbilityComp->CaptureAttributeForGameplayEffect(CaptureSpec);
    FAggregatorEvaluateParameters Parameters;
    FGameplayTagContainer OwnedTags;
    GetOwnedGameplayTags(OwnedTags);
    Parameters.SourceTags = &OwnedTags;
    float CapturedValue = 0.0f;
    if (CaptureSpec.AttemptCalculateAttributeMagnitudeWithBase(
            Parameters, static_cast<float>(StackLimit), CapturedValue))
    {
        // CVTTSS2SI yields INT_MIN for NaN/infinity/out-of-range, without
        // introducing an undefined C++ float-to-integer conversion.
        StackLimit = (!FMath::IsFinite(CapturedValue) || CapturedValue >= 2147483648.0f
                      || CapturedValue < -2147483648.0f)
            ? MIN_int32 : static_cast<int32>(CapturedValue);
    }
}
return StackLimit;
