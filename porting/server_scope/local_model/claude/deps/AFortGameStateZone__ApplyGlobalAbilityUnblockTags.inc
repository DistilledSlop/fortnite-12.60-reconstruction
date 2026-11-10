// Native 0x141ea0350 (called by GloballyUnblockAbilitiesWithTags). Only acts in ability restriction state 3.
if (AbilityRestrictionState == 3)
{
	bool bAnyChanged = false;
	for (const FGameplayTag& Tag : TagsToUnblock.GameplayTags)
	{
		bAnyChanged = UpdateTagCount(GameplayTagCounts, Tag, -1, true) || bAnyChanged;
	}
	if (bAnyChanged)
	{
		ExplicitTags.FillParentTags();
	}
	ActiveTags = ExplicitTags;
	UpdateGlobalAbilityRestrictionState();
}
TagsToUnblock.Empty();
