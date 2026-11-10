// CONFIDENCE: MEDIUM; no validate (thunk 0x142a45510 calls impl 0x14234a290 directly). Embedded ability-provider object at this+0x4c8 (vcall slot 0x10 returns the UAbilitySystemComponent). If the ASC exists, add the native drop tag (native tag singleton 0x141ed1800, +0x15d0) to a local container and apply the container to the ASC via 0x140731570 (per-tag helper 0x140731610 not read).
UAbilitySystemComponent* CarryASC = AbilityProvider.GetAbilitySystemComponent();
if (CarryASC)
{
	FGameplayTagContainer DropTags;
	DropTags.AddTag(NativeGameplayTags()->CarriedPlayerDropTag);
	ApplyGameplayTagContainerToAbilitySystem(AbilityProvider.GetAbilitySystemComponent(), DropTags, true);
}
