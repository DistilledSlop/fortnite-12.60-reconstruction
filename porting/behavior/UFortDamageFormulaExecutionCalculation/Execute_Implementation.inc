	// CONFIDENCE: low. Supports explicit magnitudes on authored effects.
	const FGameplayEffectSpec& Spec = Parameters.GetOwningSpec();
	static const FGameplayTag ExplicitDamageTag = FGameplayTag::RequestGameplayTag(
		FName(TEXT("SetByCaller.AbilityDamage")), false);
	const float NamedDamage = Spec.GetSetByCallerMagnitude(FName(TEXT("Damage")), false);
	const float Amount = ExplicitDamageTag.IsValid()
		? Spec.GetSetByCallerMagnitude(ExplicitDamageTag, false, NamedDamage)
		: NamedDamage;
	if (FMath::IsFinite(Amount) && Amount > 0.0f)
	{
		static const FGameplayAttribute Damage(FindFProperty<FProperty>(UFortHealthSet::StaticClass(), TEXT("Damage")));
		Output.AddOutputModifier(FGameplayModifierEvaluatedData(Damage, EGameplayModOp::Additive, Amount));
	}
