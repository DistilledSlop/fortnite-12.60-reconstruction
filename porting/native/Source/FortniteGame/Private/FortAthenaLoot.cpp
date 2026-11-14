#include "FortAthenaLoot.h"

#include "AFortPickupAthena.h"
#include "AFortInventory.h"
#include "AFortPlayerPawn.h"
#include "AFortPlayerController.h"
#include "FortAthenaInventoryFlow.h"
#include "UFortWorldItem.h"
#include "UObject/ScriptMacros.h"
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

namespace FortAthenaLootTransfer
{
	void InstallRpcOverrides();
	bool HandlePickup(AFortPlayerPawn* Pawn, AFortPickup* Pickup,
		float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound);
}

namespace
{
	// Delegates to FortAthenaLootTransfer for full pickup validation, stacking,
	// quickbar slot replication (DelayedQuickBarActions), swap, and flight replication.
	void ExecServerHandlePickup(UObject* Context, FFrame& Stack, RESULT_DECL)
	{
		P_GET_OBJECT(AFortPickup, Pickup);
		P_GET_PROPERTY(FFloatProperty, FlyTime);
		P_GET_STRUCT(FVector, StartDirection);
		P_GET_UBOOL(bPlayPickupSound);
		P_FINISH;
		P_NATIVE_BEGIN;
		FortAthenaLootTransfer::HandlePickup(Cast<AFortPlayerPawn>(Context), Pickup, FlyTime, StartDirection, bPlayPickupSound);
		P_NATIVE_END;
	}
}

void FortAthenaLoot::InstallRpcOverrides()
{
	if (!IsEnabled())
		return;
	FortAthenaLootTransfer::InstallRpcOverrides();
}
