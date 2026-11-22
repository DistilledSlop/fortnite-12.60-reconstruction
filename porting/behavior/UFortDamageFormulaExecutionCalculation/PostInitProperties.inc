// CONFIDENCE: medium. Register source base damage and live target resistance.
Super::PostInitProperties();
const FGameplayAttribute BaseDamage(FindFProperty<FProperty>(UFortDamageSet::StaticClass(), TEXT("OutgoingBaseDamage")));
RelevantAttributesToCapture.AddUnique(FGameplayEffectAttributeCaptureDefinition(
	BaseDamage, EGameplayEffectAttributeCaptureSource::Source, true));
const FGameplayAttribute Resistance(FindFProperty<FProperty>(UFortHealthSet::StaticClass(), TEXT("DamageResistance")));
RelevantAttributesToCapture.AddUnique(FGameplayEffectAttributeCaptureDefinition(
	Resistance, EGameplayEffectAttributeCaptureSource::Target, false));
