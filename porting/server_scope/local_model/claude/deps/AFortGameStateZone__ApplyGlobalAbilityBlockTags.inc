// Native 0x141ea02a0 (called by GloballyBlockAbilitiesWithTags). Only acts in ability restriction state 3 (this->AbilityRestrictionState at +0xf0).
if (AbilityRestrictionState == 3)
{
	for (const FGameplayTag& Tag : TagsToBlock.GameplayTags)
	{
		UpdateTagCount(GameplayTagCounts, Tag, 1, true);
	}
	ActiveTags = ExplicitTags;
	UpdateGlobalAbilityRestrictionState();
}
TagsToBlock.Empty();
