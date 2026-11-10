// CONFIDENCE: MEDIUM; no validate (thunk 0x142a46140 calls impl 0x14234ac80 directly). Same structure as ServerDropCarriedPlayer: the ability provider at this+0x4c8 must yield an ASC, then the throw tag (native tag singleton +0x15c8) is added to a local container and applied to the ASC through 0x140731570.
UAbilitySystemComponent* CarryASC = AbilityProvider.GetAbilitySystemComponent();
if (CarryASC)
{
	FGameplayTagContainer ThrowTags;
	ThrowTags.AddTag(NativeGameplayTags()->CarriedPlayerThrowTag);
	ApplyGameplayTagContainerToAbilitySystem(AbilityProvider.GetAbilitySystemComponent(), ThrowTags, true);
}
