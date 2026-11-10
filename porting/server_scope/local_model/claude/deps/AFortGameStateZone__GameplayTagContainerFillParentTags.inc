// dep AFortGameStateZone__GameplayTagContainerFillParentTags original 0x143e8f0b0
// CONFIDENCE: MEDIUM. Original 0x143e8f0b0. Rebuilds ParentTags from the explicit tags.
Container.ParentTags.Reset();
for (const FGameplayTag& Tag : Container.GameplayTags)
{
	const FGameplayTagNode* Node = GameplayTagManager::FindTagNode(Tag);   // 0x141837200
	if (Node != nullptr)
	{
		for (const FGameplayTag& Parent : Node->ParentTags)
		{
			Container.ParentTags.AddUnique(Parent);   // 0x1406a8440
		}
	}
}
