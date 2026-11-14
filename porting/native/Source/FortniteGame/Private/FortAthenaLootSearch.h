#pragma once

#include "CoreMinimal.h"

class ABuildingContainer;
class AFortGameModeAthena;
class AFortPickupAthena;
class AFortPlayerController;
class UDataTable;
class UFortItemDefinition;

// Server-side chest, ammo box, and container search flow (G8-L5).
// Original RPC chain: UFortControllerComponent_Interaction::ServerAttemptInteract (0x1428a5d20)
// -> 0x141d15810 -> 0x141d14c20 -> ABuildingContainer::ServerOnAttemptInteract (0x141be85b0).
namespace FortAthenaLootSearch
{
	// Gated behind -FortniteOriginalClient (mirrors FortAthenaLoot::IsEnabled).
	bool IsEnabled();

	// Rebinds UFortControllerComponent_Interaction::ServerAttemptInteract on the server.
	void InstallRpcOverrides();

	// Resolves a dropped item's ItemLevel from LootLevelData (category + LootLevel) clamped by
	// UFortWorldItemDefinition::MinLevel/MaxLevel. Confidence: medium (data-driven from LootLevelData).
	bool ResolveItemLevel(const UDataTable* LevelTable, UFortItemDefinition* Definition, int32 LootLevel, double Random01, int32& OutLevel);

	// Resolves initial LoadedAmmo from UFortWeaponItemDefinition::WeaponStatHandle -> FFortBaseWeaponStats::ClipSize.
	int32 ResolveLoadedAmmo(const UFortItemDefinition* Definition);

	// Resolves Container's active loot tier group, respecting ChosenRandomUpgrade if set.
	// Original accessor: 0x141beb120 (confidence: high).
	FName ResolveContainerLootTierGroup(const ABuildingContainer* Container);

	// Rolls TierGroup via FortAthenaLootRoll::RollForMode and spawns AFortPickupAthena drops around WorldOrigin.
	// Note: loot toss velocity / PickupLocationData flight is unrecovered (spawns with small radial stand-in scatter).
	bool RollAndSpawnContainerLoot(
		AFortGameModeAthena* GameMode,
		ABuildingContainer* Container,
		FName TierGroup,
		const FVector& WorldOrigin,
		float ScatterRadius,
		TArray<AFortPickupAthena*>* OutPickups = nullptr);

	// Executes the authoritative container search transition (roll, spawn, mark searched / destroy).
	// Confidence: medium (validation & state machine from 0x141d14c20 / 0x141be85b0; Roll/Spawn replace stripped 0x141bec430).
	bool TrySearchContainer(AFortPlayerController* Controller, ABuildingContainer* Container);
}
