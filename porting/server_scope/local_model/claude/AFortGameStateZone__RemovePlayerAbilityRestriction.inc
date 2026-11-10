// CONFIDENCE: MEDIUM. Original 0x14292ca10. Mirror of ApplyPlayerAbilityRestriction (0x14292a6b0) for the global ability restriction tag.
FGameplayTagContainer GlobalTags;
GameplayTagContainerFromTag(GlobalTags, GetFortGlobals()->GlobalAbilityRestrictionTag);   // 0x143e8b5c0 on FFortGlobals+0x788
FGameplayTagContainer RestrictionTags = GlobalTags;                                           // copy-assign (0x1406ba850 style, engine)
if (Role == ROLE_Authority)
{
	bool bAnyRemoved = false;
	for (const FGameplayTag& Tag : RestrictionTags.GameplayTags)
	{
		bAnyRemoved |= UpdateTagCount(GloballyBlockedAbilityTagCounts, Tag, -1, true);   // 0x140744d30
	}
	if (bAnyRemoved)
	{
		GameplayTagContainerFillParentTags(GlobalAbilityBlockTags);                       // 0x143e8f0b0
	}
	ExplicitGloballyBlockedAbilityTags = GlobalAbilityBlockTags;                          // +0x8a8 <- +0x888
	UpdateGlobalAbilityRestrictionState();                                                // 0x141ea5f80
}
