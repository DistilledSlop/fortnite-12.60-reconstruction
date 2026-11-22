	// CONFIDENCE: low. Supports explicit magnitudes and source base damage.
	const FGameplayEffectSpec& Spec = Parameters.GetOwningSpec();
	static const FGameplayTag ExplicitDamageTag = FGameplayTag::RequestGameplayTag(
		FName(TEXT("SetByCaller.AbilityDamage")), false);
	const float* ExplicitDamage = Spec.SetByCallerTagMagnitudes.Find(ExplicitDamageTag);
	if (!ExplicitDamage)
	{
		ExplicitDamage = Spec.SetByCallerNameMagnitudes.Find(FName(TEXT("Damage")));
	}

	float Amount = 0.0f;
	if (ExplicitDamage)
	{
		Amount = *ExplicitDamage;
	}
	else
	{
		FAggregatorEvaluateParameters Evaluation;
		Evaluation.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
		Evaluation.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
		static const FGameplayEffectAttributeCaptureDefinition BaseDamage(
			FGameplayAttribute(FindFProperty<FProperty>(UFortDamageSet::StaticClass(), TEXT("OutgoingBaseDamage"))),
			EGameplayEffectAttributeCaptureSource::Source, true);
		Parameters.AttemptCalculateCapturedAttributeMagnitude(BaseDamage, Evaluation, Amount);
	}
	if (FMath::IsFinite(Amount) && Amount > 0.0f)
	{
		static const FGameplayAttribute Damage(FindFProperty<FProperty>(UFortHealthSet::StaticClass(), TEXT("Damage")));
		Output.AddOutputModifier(FGameplayModifierEvaluatedData(Damage, EGameplayModOp::Additive, Amount));
	}
