// CONFIDENCE: MEDIUM. Allocates a gameplay effect context carrying the Fort source data: instigator (causer, falling back to the owner actor), damage source and source object. The level argument is accepted for the Blueprint signature but is not consumed by the implementation.
// Original addresses: exec 0x142781640; body helper 0x141604940; context alloc via 0x1406c6ef0.
FGameplayEffectContextHandle ReturnValue = UAbilitySystemGlobals::Get().AllocGameplayEffectContext();

AActor* Causer = EffectCauser ? EffectCauser : AbilityActorInfo->OwnerActor.Get();
ReturnValue.AddInstigator(AbilityActorInfo->AvatarActor.Get(), Causer);

if (DamageSource)
{
	if (IFortDamageSourceInterface* DamageSourceIface = Cast<IFortDamageSourceInterface>(DamageSource))
	{
		ReturnValue.SetDamageSource(DamageSourceIface);
	}
}
if (SourceObject)
{
	ReturnValue.Get()->SetSourceObject(SourceObject);
}
return ReturnValue;
