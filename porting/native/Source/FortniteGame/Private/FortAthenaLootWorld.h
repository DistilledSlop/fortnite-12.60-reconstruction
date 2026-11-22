#pragma once

#include "CoreMinimal.h"

class AFortGameModeAthena;
class ULevel;

// Server-side floor loot population pass for loaded and streamed levels (G8-L4).
// Identifies floor-loot marker containers (Tiered_Athena_FloorLoot_01_C, Tiered_Athena_FloorLoot_Warmup_C)
// by bStartAlreadySearched_Athena and non-empty SearchLootTierGroup, rolls once per actor via
// FortAthenaLootRoll / FortAthenaLootSearch, and spawns AFortPickupAthena at the marker transform.
// A map-authored actor tag LootDeliverySeconds=<delay> schedules the marker relative to the
// start of the safe-zone phase. The marker needs an assigned loot tier group.
// Add LootDeliveryCrate to a marker with a movable root to descend to the authored transform
// and unlock the container for normal search on landing. Without a movable root,
// the marker spawns pickups at its authored transform when the delivery is due.
// GroundLootCacheCandidate markers are selected randomly from visible levels when play starts.
// GroundLootCacheCount caps activations per match (default one) as candidate levels become visible.
// Add BreakableLootContainer to enable lethal-hit opening.
namespace FortAthenaLootWorld
{
	// Gated behind -FortniteOriginalClient.
	bool IsEnabled();

	// Clears per-world tracking state.
	void Reset();

	// Scans any visible levels whose floor-loot markers have not yet been populated once loot data is ready.
	// Call from FortAthenaMatchFlow::DriverTick.
	void Tick(AFortGameModeAthena* GameMode);

	// Populates markers in a newly added/streamed ULevel if visible and loot data is ready.
	// Call from FWorldDelegates::LevelAddedToWorld in FortAthenaMatchFlow.
	void OnLevelAdded(AFortGameModeAthena* GameMode, ULevel* Level);

	// Populates unprocessed floor-loot markers in OptionalLevel (or all world levels if OptionalLevel == nullptr).
	// Returns the number of newly populated markers.
	int32 PopulateLoadedMarkers(AFortGameModeAthena* GameMode, ULevel* OptionalLevel = nullptr);
}
