// CONFIDENCE: LOW. Chunk 01 port. See ABuildingGameplayActorConsumable__MakeOnConsumeGameplayEffectSpec.evidence.json for captures and notes.
// Original address: 0x142744000 (exec). Body 0x141be2e10 (parameter order EventCauser at offset 0, Level at offset 8, matching reflection).
FGameplayEffectSpecHandle SpecHandle;

UAbilitySystemComponent* AbilitySystem = GetGameStateAbilitySystemComponent();
TSubclassOf<UGameplayEffect> EffectClass = DetermineOnConsumeGameplayEffect();
if (AbilitySystem && EffectClass && EffectClass->IsChildOf(UGameplayEffect::StaticClass()))
{
	FGameplayEffectContextHandle Context = MakeEffectContextForConsumable(EventCauser);
	SpecHandle = AbilitySystem->MakeOutgoingSpec(EffectClass, Level, Context);
	if (SpecHandle.IsValid())
	{
		PrepareOnConsumeGameplayEffectSpec(SpecHandle);
	}
}

return SpecHandle;
