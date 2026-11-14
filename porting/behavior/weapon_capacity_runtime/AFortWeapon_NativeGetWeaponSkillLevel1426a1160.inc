// CONFIDENCE: HIGH. Complete body; shared native state is declared in field proposals.
if (bNativeWeaponSkillCached201)
    return NativeCachedWeaponSkill201;
int32 Level = NativeGetWeaponLevel140696560();
if (PawnInterface)
{
    if (UAbilitySystemComponent* OwnerASC = PawnInterface->GetAbilitySystemComponent())
    {
        FGameplayTagContainer WeaponTags;
        if (WeaponData)
            WeaponData->GetOwnedGameplayTags(WeaponTags);
        FGameplayTagContainer SourceTags;
        OwnerASC->GetOwnedGameplayTags(SourceTags);
        SourceTags.AppendTags(WeaponTags);
        FAggregatorEvaluateParameters Parameters;
        Parameters.SourceTags = &SourceTags;
        if (const UFortDamageSet* DamageSet = OwnerASC->GetSet<UFortDamageSet>())
        {
            float Skill;
            {
                static FProperty* SkillProperty = FindFieldChecked<FProperty>(
                    UFortDamageSet::StaticClass(), FName(TEXT("WeaponSkill")));
                static const FGameplayEffectAttributeCaptureDefinition Definition(
                    FGameplayAttribute(SkillProperty), EGameplayEffectAttributeCaptureSource::Source, true);
                FGameplayEffectAttributeCaptureSpec Spec(Definition);
                OwnerASC->CaptureAttributeForGameplayEffect(Spec);
                Skill = 0.0f;
                Spec.AttemptCalculateAttributeMagnitude(Parameters, Skill);
                // Original COMISS/JNC clamps unordered input to Minimum.
                if (!_mm_comige_ss(_mm_set_ss(Skill), _mm_set_ss(DamageSet->WeaponSkill.Minimum)))
                    Skill = DamageSet->WeaponSkill.Minimum;
                else if (_mm_comige_ss(_mm_set_ss(Skill), _mm_set_ss(DamageSet->WeaponSkill.Maximum)))
                    Skill = DamageSet->WeaponSkill.Maximum;
            }
            const __m128 V = _mm_set_ss(Skill);
            const int32 Rounded = _mm_cvtss_si32(_mm_sub_ss(_mm_add_ss(V, V), _mm_set_ss(0.5f))) >> 1;
            if (Rounded <= Level)
                Level = Rounded;
        }
    }
}
if (NativeStatCacheScopeDepth201 > 0)
{
    NativeCachedWeaponSkill201 = Level;
    bNativeWeaponSkillCached201 = true;
}
return Level;
