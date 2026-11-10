// dep AFortGameStateZone__GameplayTagContainerFromTag original 0x143e8b5c0
// CONFIDENCE: MEDIUM. Original 0x143e8b5c0. Adds Tag to Container.GameplayTags, then adds Tag's parent tags (from the tag manager node) to Container.ParentTags with AddUnique.
Container.GameplayTags.Add(Tag);
const FGameplayTagNode* Node = GameplayTagManager::FindTagNode(Tag);   // 0x141837200 on the manager at _DAT_148323420+0x60
if (Node != nullptr)
{
	for (const FGameplayTag& Parent : Node->ParentTags)
	{
		Container.ParentTags.AddUnique(Parent);   // 0x1406a8440
	}
}
