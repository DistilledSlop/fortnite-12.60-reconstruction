// Native 0x140744d30: FGameplayTagCountContainer-style update, used by the four ability-restriction paths with Delta +1 (block/apply) or -1 (unblock). Returns true when a tag's count changed across zero (notification-worthy).
// CONFIDENCE: LOW. Offsets are from the decompile: direct count map +0xa0 and parent-tag count map +0x50, explicit tag list +0x108 (direct) and +0x118 (parent), invocation list +0xf0 (container) and tag-node invocation list at node+0x18. Parent walk uses the tag-node table (helper 0x14073de00). Helper bodies 0x1406e7be0, 0x143e8b3b0, 0x143e96610, 0x143e904d0 and 0x140739eb0 are not ported line by line; their semantics are AddOrFind, AddExplicit, RemoveExplicit, GetParentTags and NotifyChange.
bool bChanged = false;
int32& Count = DirectCounts.FindOrAdd(Tag);
int32 Previous = Count;
Count = FMath::Max(Count + Delta, 0);
if (Previous == 0 && Count > 0)
{
	ExplicitTags.AddUnique(Tag);
	bChanged = true;
}
if (Count < 1)
{
	ExplicitTags.Remove(Tag);
	CompactInvocationList(DirectChangeDelegates, false);
	bChanged = true;
}
for (const FGameplayTag& ParentTag : Tag.GetParentTags())
{
	int32& ParentCount = ParentCounts.FindOrAdd(ParentTag);
	int32 ParentPrevious = ParentCount;
	ParentCount = FMath::Max(ParentCount + Delta, 0);
	if (ParentPrevious == 0 || ParentCount == 0)
	{
		CompactInvocationList(ParentChangeDelegates, false);
	}
}
return bChanged;
