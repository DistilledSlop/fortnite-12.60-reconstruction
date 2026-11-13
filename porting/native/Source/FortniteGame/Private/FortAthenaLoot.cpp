#include "FortAthenaLoot.h"

#include "AFortPickupAthena.h"
#include "UFortItemDefinition.h"
#include "Engine/World.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

bool FortAthenaLoot::IsEnabled()
{
	return FParse::Param(FCommandLine::Get(), TEXT("FortniteOriginalClient"));
}

AFortPickupAthena* FortAthenaLoot::SpawnResolvedPickup(UWorld* World, const FVector& Location,
	UFortItemDefinition* Definition, int32 Count, int32 Level, int32 LoadedAmmo)
{
	if (!IsEnabled() || !World || World->GetNetMode() == NM_Client || !Definition || Count <= 0)
		return nullptr;

	// stand-in: native pickup selection, initialization and toss behavior still need recovery.
	// The result is not connected to a table roll or a map spawner yet.
	FActorSpawnParameters Parameters;
	Parameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AFortPickupAthena* Pickup = World->SpawnActor<AFortPickupAthena>(Location, FRotator::ZeroRotator, Parameters);
	if (!Pickup)
	{
		UE_LOG(LogTemp, Warning, TEXT("Loot: failed to spawn resolved pickup"));
		return nullptr;
	}
	Pickup->PrimaryPickupItemEntry.ItemDefinition = Definition;
	Pickup->PrimaryPickupItemEntry.Count = Count;
	Pickup->PrimaryPickupItemEntry.Level = Level;
	Pickup->PrimaryPickupItemEntry.LoadedAmmo = LoadedAmmo;
	Pickup->PrimaryPickupItemEntry.ItemGuid = FGuid::NewGuid();
	Pickup->SetReplicates(true);
	Pickup->ForceNetUpdate();
	UE_LOG(LogTemp, Display, TEXT("Loot: spawned resolved pickup %s count=%d"), *Definition->GetName(), Count);
	return Pickup;
}

void FortAthenaLoot::InstallRpcOverrides()
{
	if (!IsEnabled())
		return;
	// stand-in: no RPC override is installed until pickup and drop semantics are recovered.
	static bool bReported = false;
	if (!bReported)
	{
		bReported = true;
		UE_LOG(LogTemp, Warning, TEXT("Loot: table rolling and pickup/drop RPCs are not implemented"));
	}
}
