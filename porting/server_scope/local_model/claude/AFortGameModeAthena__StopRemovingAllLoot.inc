// CONFIDENCE: MEDIUM
// Original addresses: exec 0x1428f0970 (args WorldContext, bImmediateCleanup) -> helper 0x141972f90.
// Loot-removal timer lives on the game mode at +0x1570 (FTimerHandle), status read from the timer data byte.
UWorld* World = GetWorldFromContextObject(WorldContext);     // 0x144640410(_DAT_148351eb8, WorldContext, 1)
AFortGameModeAthena* GameMode = (World != nullptr) ? World->AuthorityGameMode : nullptr;   // World+0x128
if (GameMode != nullptr && GameMode->IsA(AFortGameModeAthena::StaticClass()))   // 0x1428ee970
{
	FTimerHandle LootTimerHandle = GameMode->LootTimerHandle;   // +0x1570 copy
	FTimerManager& TimerManager = GetWorldTimerManager(GameMode);   // 0x140f95ae0 (world->GetTimerManager with fallback)
	// Timer is considered active when it exists and its status byte is not 2 (the value used for "not running" in 0x144621fb0 callers).
	bool bLootTimerActive = false;
	if (FTimerData* Timer = FindTimer(TimerManager, LootTimerHandle))   // 0x144621fb0
	{
		bLootTimerActive = (Timer->Status != kLootTimerStoppedStatus);   // status byte at +1, value 2
	}

	if (bLootTimerActive)
	{
		if (bImmediateCleanup)
		{
			// Immediate full cleanup: unlimited containers and pickups, concurrent, no delay.
			StartRemovingAllLoot(GameMode, -1, -1, -1.0f, true);   // 0x14195e100
		}
		FTimerManager& TM2 = GetWorldTimerManager(GameMode);
		if (FindTimer(TM2, GameMode->LootTimerHandle) != nullptr)     // re-find through the same handle
		{
			FTimerManager_InternalClearTimer(TM2, LootTimerHandle);   // engine helper
		}
		GameMode->LootTimerHandle = FTimerHandle();                  // reset to 0
	}
}
