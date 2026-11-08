// Original address: 0x141655090 (thunk 0x142798020)
// Evidence: [this+0xa38] = ExternalAISpawners (TMap<TWeakObjectPtr<AActor>, FFortAISpawnerData>, element stride 0x70, value at element+8). Key = weak pointer built from EnemySpawner (0x142ee3b30).
// Found value's ReservedSpawnRequest.EnemyIndex (value+0x20 = FFortSpawnAIRequest at +0x10, EnemyIndex at +0x10) != -1. Null spawner or missing key returns false.
// The slot search (0x141656140) is expressed as TMap::Find on the same key.
// CONFIDENCE: MEDIUM
if (!EnemySpawner)
{
	return false;
}

const FFortAISpawnerData* SpawnerData = ExternalAISpawners.Find(TWeakObjectPtr<AActor>(EnemySpawner));
return SpawnerData != nullptr && SpawnerData->ReservedSpawnRequest.EnemyIndex != -1;
