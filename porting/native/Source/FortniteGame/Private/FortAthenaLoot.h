#pragma once

#include "CoreMinimal.h"

class AFortPickupAthena;
class UFortItemDefinition;
class UWorld;

namespace FortAthenaLoot
{
	bool IsEnabled();

	// Spawn one already-resolved loot result on the authoritative server. Table selection and
	// item resolution are intentionally separate; they have not yet been recovered here.
	AFortPickupAthena* SpawnResolvedPickup(UWorld* World, const FVector& Location,
		UFortItemDefinition* Definition, int32 Count, int32 Level, int32 LoadedAmmo);

	// No loot RPC is rebound until its complete handling contract is implemented.
	void InstallRpcOverrides();
}
