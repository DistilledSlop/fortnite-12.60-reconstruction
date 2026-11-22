	// CONFIDENCE: low. Supports explicit magnitudes and source base damage.
	const FGameplayEffectSpec& Spec = Parameters.GetOwningSpec();
	static const FGameplayTag ExplicitDamageTag = FGameplayTag::RequestGameplayTag(
		FName(TEXT("SetByCaller.AbilityDamage")), false);
	const float* ExplicitDamage = Spec.SetByCallerTagMagnitudes.Find(ExplicitDamageTag);
	if (!ExplicitDamage)
	{
		ExplicitDamage = Spec.SetByCallerNameMagnitudes.Find(FName(TEXT("Damage")));
	}

	FAggregatorEvaluateParameters Evaluation;
	Evaluation.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	Evaluation.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	float Amount = 0.0f;
	if (ExplicitDamage)
	{
		Amount = *ExplicitDamage;
	}
	else
	{
		static const FGameplayEffectAttributeCaptureDefinition BaseDamage(
			FGameplayAttribute(FindFProperty<FProperty>(UFortDamageSet::StaticClass(), TEXT("OutgoingBaseDamage"))),
			EGameplayEffectAttributeCaptureSource::Source, true);
		Parameters.AttemptCalculateCapturedAttributeMagnitude(BaseDamage, Evaluation, Amount);
	}
	static const FGameplayEffectAttributeCaptureDefinition Resistance(
		FGameplayAttribute(FindFProperty<FProperty>(UFortHealthSet::StaticClass(), TEXT("DamageResistance"))),
		EGameplayEffectAttributeCaptureSource::Target, false);
	float ResistanceValue = 0.0f;
	Parameters.AttemptCalculateCapturedAttributeMagnitude(Resistance, Evaluation, ResistanceValue);
	if (FMath::IsFinite(ResistanceValue))
	{
		Amount *= 1.0f - FMath::Clamp(ResistanceValue, 0.0f, 0.95f);
	}

	if (FMath::IsFinite(Amount) && Amount > 0.0f)
	{
		static const FGameplayAttribute Damage(FindFProperty<FProperty>(UFortHealthSet::StaticClass(), TEXT("Damage")));
		Output.AddOutputModifier(FGameplayModifierEvaluatedData(Damage, EGameplayModOp::Additive, Amount));
	}
