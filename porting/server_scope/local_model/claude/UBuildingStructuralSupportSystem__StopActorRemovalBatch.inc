// CONFIDENCE: LOW. Closes one batching level. On the outermost close, it applies all queued building removals group by group: for each removed building that has a logical-building index, it gathers the dependent building indices in the same group and removes each one once (de-duplicated per group).
// Original addresses: exec 0x14275eac0; helpers 0x141c65050 (dependent gather), 0x141c89630 (per-building removal), 0x140769940 (sparse-set copy), 0x140782160 (set Find), 0x140687d00 (bit iterator advance).
// The inner helpers are only partly recovered; their names here are proposed and the stat scope is the STAT_Fort_AuthoritativeBatchRemoveBuildingActorFromLogicalBuilding counter.
if (!GBuildingRemovalBatchingEnabled)
{
	return;
}
if (--ActorRemovalBatchDepth != 0)
{
	return;
}

SCOPE_CYCLE_COUNTER(STAT_Fort_AuthoritativeBatchRemoveBuildingActorFromLogicalBuilding);
for (int32 GroupIdx : PendingRemovalGroups)
{
	// Copy of this group's queued removals (sparse set copy).
	const TSparseArray<FWeakObjectPtr> QueuedRemovals = PendingRemovalBatches[GroupIdx].QueuedActors;
	// Building indices already removed in this group; reset whenever the group changes.
	TSet<int32> RemovedBuildingIndices;

	for (const FWeakObjectPtr& WeakRemoved : QueuedRemovals)
	{
		ABuildingSMActor* Removed = Cast<ABuildingSMActor>(WeakRemoved.Get());
		if (!Removed || Removed->LogicalBuildingIndex == INDEX_NONE)
		{
			continue;
		}

		TArray<int32> Dependents;
		GatherDependentBuildingIndices(QueuedRemovals, Removed->LogicalBuildingIndex, Dependents);   // helper 0x141c65050
		for (int32 DependentIdx : Dependents)
		{
			if (!RemovedBuildingIndices.Contains(DependentIdx))
			{
				RemoveBuildingFromLogicalBuilding(Removed->LogicalBuildingIndex, DependentIdx, RemovedBuildingIndices);   // helper 0x141c89630
			}
		}
	}
}
