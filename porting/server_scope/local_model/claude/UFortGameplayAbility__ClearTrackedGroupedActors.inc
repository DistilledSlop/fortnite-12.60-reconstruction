// CONFIDENCE: MEDIUM Authority-only. Ignores the call when GroupTag is None. Otherwise finds the tracked actor group keyed by GroupTag (hash via FUN_142ce11c0) on the owning actor, destroys its actors when bDestroyActors is set, then removes the group.
// Original: exec 0x1428f7f20 (decompile captures/0da226f5ed8442fb9ce91e4985f6c96a/0x1428f7f20.c)
if (!HasAuthority(&CurrentActivationInfo))
{
	return;
}
if (!GroupTag.IsValid())
{
	return;
}
ClearTrackedActorGroupByTag(GroupTag, bDestroyActors);
