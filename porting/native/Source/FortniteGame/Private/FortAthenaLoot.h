#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"

class AFortPickup;
class AFortPlayerController;
class UFortItemDefinition;
class UWorld;

namespace FortAthenaLoot
{
	bool IsEnabled();

	// Spawn one already-resolved loot result on the authoritative server. Table selection and
	// item resolution are intentionally separate; they have not yet been recovered here.
	AFortPickup* SpawnResolvedPickup(UWorld* World, const FVector& Location,
		UFortItemDefinition* Definition, int32 Count, int32 Level, int32 LoadedAmmo, const FVector* TossFrom = nullptr,
		AFortPlayerController* Owner = nullptr, bool bOnlyRelevantToOwner = false, TSubclassOf<AFortPickup> PickupClass = nullptr);

	// Bind the limited immediate-collection pickup RPC; swap, drop and search remain unimplemented.
	void InstallRpcOverrides();
}
