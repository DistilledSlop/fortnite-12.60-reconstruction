// DRAFT - UNCOMPILED, UNCOUNTED. Native 0x1415ec3f0, wrapper/exec 0x1427807f0 (reflected, flags 0x4020401).
// Reflected params in wrapper step order: GameplayEffectClass, TargetOfEffect, OptionalSourceObject, OptionalEffectCauser,
//   OptionalDamageSource, Level, EffectContext(by value) -> FActiveGameplayEffectHandle.
// Fort context: UFortAbilitySystemGlobals::AllocGameplayEffectContext (0x1415e9820) returns FFortGameplayEffectContext (0xd8); SetDamageSource = 0x14160d0f0
// (both recovered in FFortGameplayEffectContext.draft.inc/.h). Original 0x1415ec60c..61a converts Level via cvttss2si/cvtdq2ps.
// Engine bindings proven by dep bodies: 0x1406dd8f0 = FGameplayEffectContextHandle::ToString ("NONE" if no data, else Data->ToString vslot 0x98);
//   0x142ec9730 = UObject::GetFullName; 0x1406c6ef0 = IGameplayAbilitiesModule::Get().GetAbilitySystemGlobals(); globals vslot 0x270 = AllocGameplayEffectContext;
//   0x1406ab0b0 = FGameplayEffectContextHandle(FGameplayEffectContext*); Context vslot 0x10 = AddInstigator, 0x48 = AddSourceObject (both vs Engine vtable order);
//   0x1406b7f10 = UAbilitySystemComponent::ApplyGameplayEffectToTarget (HasNetworkAuthorityToApplyGameplayEffect vslot 0x598; MakeEffectContext vslot 0x5d8; FGameplayEffectSpec ctor 0x1406ecca0; ApplyGameplayEffectSpecToTarget vslot 0x5b0).

FActiveGameplayEffectHandle UFortAbilitySystemComponent::BP_FortApplyGameplayEffectToTarget(
	TSubclassOf<UGameplayEffect> GameplayEffectClass,
	UAbilitySystemComponent* TargetOfEffect,
	UObject* OptionalSourceObject,
	AActor* OptionalEffectCauser,
	AActor* OptionalDamageSource,
	float Level,
	FGameplayEffectContextHandle EffectContext)
{
	if (TargetOfEffect == nullptr)
	{
		// guarded by verbosity > 4 (Log)
		ABILITY_LOG(Log, TEXT("UFortAbilitySystemComponent::BP_FortApplyGameplayEffectToTarget called with null TargetOfEffect. %s. EffectContext: %s"),
			*GetFullName(), *EffectContext.ToString());
		return FActiveGameplayEffectHandle();   // Handle = INDEX_NONE (-1), bPassedFiltersAndWasExecuted = false
	}

	if (GameplayEffectClass == nullptr || !GameplayEffectClass->IsChildOf(UGameplayEffect::StaticClass()))   // 0x140774520 = UGameplayEffect::StaticClass
	{
		// guarded by verbosity > 1 (Error)
		ABILITY_LOG(Error, TEXT("UFortAbilitySystemComponent::BP_FortApplyGameplayEffectToTarget called with null GameplayEffectClass. %s. EffectContext: %s"),
			*GetFullName(), *EffectContext.ToString());
		return FActiveGameplayEffectHandle();
	}

	if (!EffectContext.IsValid())   // param_9[1] (Data) == 0
	{
		// 0x1406c6ef0 + vslot 0x270 + 0x1406ab0b0
		FGameplayEffectContextHandle NewContext(UAbilitySystemGlobals::Get().AllocGameplayEffectContext());
		AActor* Causer = OptionalEffectCauser;
		if (Causer == nullptr)
		{
			Causer = AbilityActorInfo->AvatarActor.Get();   // weak @ actorinfo(+0x3e8)+0x10
		}
		AActor* Instigator = AbilityActorInfo->OwnerActor.Get();   // weak @ actorinfo+8
		NewContext.AddInstigator(Instigator, Causer);              // ctx vslot 0x10

		FGameplayEffectContext* Data = NewContext.Get();
		if (Data != nullptr)
		{
			if (OptionalDamageSource != nullptr)
			{
				// 0x1428cead0 interface class, 0x142ec9900 lookup
				if (IFortDamageSourceInterface* Interface = static_cast<IFortDamageSourceInterface*>(OptionalDamageSource->GetInterfaceAddress(UFortDamageSourceInterface::StaticClass())))
				{
					static_cast<FFortGameplayEffectContext*>(Data)->SetDamageSource(Interface, static_cast<float>(static_cast<int32>(Level)));   // 0x14160d0f0
				}
			}
			if (OptionalSourceObject != nullptr)
			{
				Data->AddSourceObject(OptionalSourceObject);   // ctx vslot 0x48
			}
		}
		EffectContext = NewContext;
	}

	UGameplayEffect* GameplayEffect = GameplayEffectClass->GetDefaultObject<UGameplayEffect>();   // class+0x130; vslot 0x390 creates if null
	return ApplyGameplayEffectToTarget(GameplayEffect, TargetOfEffect, Level, EffectContext, FPredictionKey());   // 0x1406b7f10
}
