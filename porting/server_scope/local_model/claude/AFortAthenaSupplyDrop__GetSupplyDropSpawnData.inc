// CONFIDENCE: MEDIUM. Original address: 0x142833eb0 (exec thunk). Helper: 0x1419189c0 (captured, read in full).
// Behaviour: the supply drop spawn height range comes from the authority GameMode's supply drop data when it is a
// FortGameModeAthena (0x141949a30). Otherwise the first registered SupplyDropSpawner supplies it. Fallback 28000 / 32000.
MinSpawnHeight = 28000.0f;
MaxSpawnHeight = 32000.0f;

if (UWorld* World = GetWorld())
{
    AGameModeBase* GameMode = World->AuthorityGameMode;
    if (GameMode != nullptr && GameMode->IsA(AFortGameModeAthena::StaticClass()))
    {
        GameMode->GetSupplyDropSpawnRange(this, MinSpawnHeight, MaxSpawnHeight);   // FUN_141949a30
        return;
    }

    TArray<AActor*> Spawners;
    FindAllActorsOfClass(World, USupplyDropSpawner::StaticClass(), Spawners);   // FUN_144212e10
    if (Spawners.Num() > 0 && Spawners[0] != nullptr)
    {
        AActor* Spawner = Spawners[0];
        if (Spawner->IsA(USupplyDropSpawner::StaticClass()) && IsObjectRegistered(Spawner))
        {
            MinSpawnHeight = Spawner->SpawnHeightMin;   // +0xb98
            MaxSpawnHeight = Spawner->SpawnHeightMax;   // +0xb9c
        }
    }
}
