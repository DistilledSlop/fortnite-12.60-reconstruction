// CONFIDENCE: MEDIUM. Blueprint entry point for applying a gameplay effect with Fort-specific source data. Builds the effect context on first use (instigator, damage source, source object), then applies the effect class default object at the given level through the base ApplyGameplayEffectToTarget.
// Original addresses: exec 0x1427807f0; body helper 0x1415ec3f0 (-> base apply 0x1406b7f10).
if (!TargetOfEffect)
{
	UE_LOG(LogTemp, Warning, TEXT("UFortAbilitySystemComponent::BP_FortApplyGameplayEffectToTarget called with null TargetOfEffect. %s. EffectContext: %s"), *GetPathName(this), *EffectContext.ToString());
	return FActiveGameplayEffectHandle();
}
if (!GameplayEffectClass || !GameplayEffectClass->IsChildOf(UGameplayEffect::StaticClass()))
{
	UE_LOG(LogTemp, Warning, TEXT("UFortAbilitySystemComponent::BP_FortApplyGameplayEffectToTarget called with null GameplayEffectClass. %s. EffectContext: %s"), *GetPathName(this), *EffectContext.ToString());
	return FActiveGameplayEffectHandle();
}

if (!EffectContext.IsValid())
{
	EffectContext = UAbilitySystemGlobals::Get().AllocGameplayEffectContext();
	if (!OptionalEffectCauser)
	{
		OptionalEffectCauser = AbilityActorInfo->OwnerActor.Get();
	}
	EffectContext.AddInstigator(AbilityActorInfo->AvatarActor.Get(), OptionalEffectCauser);
	if (OptionalDamageSource)
	{
		if (IFortDamageSourceInterface* DamageSourceIface = Cast<IFortDamageSourceInterface>(OptionalDamageSource))
		{
			EffectContext.SetDamageSource(DamageSourceIface);
		}
	}
	if (OptionalSourceObject)
	{
		EffectContext.Get()->SetSourceObject(OptionalSourceObject);
	}
}

UGameplayEffect* GameplayEffectCDO = GameplayEffectClass->GetDefaultObject<UGameplayEffect>();
return ApplyGameplayEffectToTarget(GameplayEffectCDO, TargetOfEffect, Level, EffectContext);
