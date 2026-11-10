// CONFIDENCE: HIGH
// Original addresses: exec 0x1428f0750 (params read in order WorldContext, ContainersPerIteration, PickupsPerIteration,
// TimeBetweenIterations, bProcessContainersAndPickupsConcurrently) -> helper 0x14195e060 -> StartRemovingAllLoot 0x14195e100.
// Declared order confirmed from the thunk read order and the reflection offsets (0, 8, 12, 16, bool).
if (WorldContext != nullptr)
{
	UWorld* World = GetWorldFromContextObject(WorldContext);     // 0x144640410(_DAT_148351eb8, WorldContext, 1)
	AFortGameModeAthena* GameMode = (World != nullptr) ? World->AuthorityGameMode : nullptr;   // World+0x128
	if (GameMode != nullptr && GameMode->IsA(AFortGameModeAthena::StaticClass()))   // 0x1428ee970
	{
		StartRemovingAllLoot(GameMode, ContainersPerIteration, PickupsPerIteration, TimeBetweenIterations, bProcessContainersAndPickupsConcurrently);   // 0x14195e100
	}
}
