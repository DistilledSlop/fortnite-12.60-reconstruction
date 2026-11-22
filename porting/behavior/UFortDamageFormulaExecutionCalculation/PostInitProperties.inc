// CONFIDENCE: medium. Register the outgoing base damage snapshot for this execution.
Super::PostInitProperties();
const FGameplayAttribute BaseDamage(FindFProperty<FProperty>(UFortDamageSet::StaticClass(), TEXT("OutgoingBaseDamage")));
RelevantAttributesToCapture.AddUnique(FGameplayEffectAttributeCaptureDefinition(
	BaseDamage, EGameplayEffectAttributeCaptureSource::Source, true));
