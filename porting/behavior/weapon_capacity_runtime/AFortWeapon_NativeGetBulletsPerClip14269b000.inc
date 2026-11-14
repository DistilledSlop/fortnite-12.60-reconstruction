// CONFIDENCE: HIGH. Complete body; shared native state is declared in field proposals.
if (bNativeClipSizeCached201)
    return NativeCachedClipSize201;
const FFortBaseWeaponStats* Stats = NativeGetWeaponStats();
if (!Stats)
    return 0;
AFortPawn* Pawn = Cast<AFortPawn>(GetInstigator());
IAbilitySystemInterface* PawnInterface = Pawn;
UAbilitySystemComponent* OwnerASC = PawnInterface ? PawnInterface->GetAbilitySystemComponent() : nullptr;
// Both the side effects and return matter: original XMM1 carries this level to 142669d00.
const int32 Level = NativeGetWeaponSkillLevel1426a1160(PawnInterface);
const int32 BaseClipSize = UFortWeaponItemDefinition::NativeScaledClipSize142669d00(*Stats, static_cast<float>(Level));
if (!OwnerASC)
    return _mm_cvttss_si32(_mm_set_ss(static_cast<float>(BaseClipSize)));
static FProperty* ClipProperty = FindFieldChecked<FProperty>(
    UFortWeaponAttrSet::StaticClass(), FName(TEXT("WeaponAmmoClipSize")));
static const FGameplayEffectAttributeCaptureDefinition Definition(
    FGameplayAttribute(ClipProperty), EGameplayEffectAttributeCaptureSource::Source, true);
FGameplayEffectAttributeCaptureSpec Spec(Definition);
OwnerASC->CaptureAttributeForGameplayEffect(Spec);
FGameplayTagContainer WeaponTags;
if (WeaponData)
    WeaponData->GetOwnedGameplayTags(WeaponTags);
FGameplayTagContainer SourceTags;
OwnerASC->GetOwnedGameplayTags(SourceTags);
SourceTags.AppendTags(WeaponTags);
FAggregatorEvaluateParameters Parameters;
Parameters.SourceTags = &SourceTags;
float ClipSize = 0.0f;
Spec.AttemptCalculateAttributeMagnitudeWithBase(Parameters, static_cast<float>(BaseClipSize), ClipSize);
const auto Round = [](float Value) {
    return _mm_cvtss_si32(_mm_sub_ss(_mm_mul_ss(_mm_set_ss(Value), _mm_set_ss(2.0f)), _mm_set_ss(0.5f))) >> 1;
};
if (NativeStatCacheScopeDepth201 > 0)
{
    bNativeClipSizeCached201 = true;
    NativeCachedClipSize201 = Round(ClipSize);
}
return Round(ClipSize);
