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
#include "EFortPickupTossState.h"
#include "TimerManager.h"
#include "GameFramework/ProjectileMovementComponent.h"

namespace FortAthenaLootTransfer
{
	void InitializeWorldPickup(AFortPickupAthena* Pickup, const FVector& Location);
}

bool FortAthenaLoot::IsEnabled()
{
	return FParse::Param(FCommandLine::Get(), TEXT("FortniteOriginalClient"));
}

AFortPickupAthena* FortAthenaLoot::SpawnResolvedPickup(UWorld* World, const FVector& Location,
	UFortItemDefinition* Definition, int32 Count, int32 Level, int32 LoadedAmmo, const FVector* TossFrom)
{
	if (!IsEnabled() || !World || World->GetNetMode() == NM_Client || !Definition || Count <= 0)
		return nullptr;

	// stand-in: native pickup selection, initialization and toss behavior still need recovery.
	// The result is not connected to a table roll or a map spawner yet.
	FActorSpawnParameters Parameters;
	Parameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	const FVector SpawnAt = TossFrom ? *TossFrom : Location;
	AFortPickupAthena* Pickup = World->SpawnActor<AFortPickupAthena>(SpawnAt, FRotator::ZeroRotator, Parameters);
	if (!Pickup)
	{
		UE_LOG(LogTemp, Warning, TEXT("Loot: failed to spawn resolved pickup"));
		return nullptr;
	}
	// The generated pickup has no root component (see InitializeWorldPickup): give it one at the requested location first.
	FortAthenaLootTransfer::InitializeWorldPickup(Pickup, SpawnAt);
	Pickup->PrimaryPickupItemEntry.ItemDefinition = Definition;
	Pickup->PrimaryPickupItemEntry.Count = Count;
	Pickup->PrimaryPickupItemEntry.Level = Level;
	Pickup->PrimaryPickupItemEntry.LoadedAmmo = LoadedAmmo;
	Pickup->PrimaryPickupItemEntry.ItemGuid = FGuid::NewGuid();
	if (TossFrom)
	{
		// Same replicated toss the dropped-item path uses (SpawnTossedPickupFromEntry): the client plays the arc from the start
		// to the rest position, the server moves the actor to the rest position when the flight ends and marks it AtRest.
		constexpr float TossFlyTime = 0.75f;
		FVector Dir = (Location - *TossFrom).GetSafeNormal();
		if (Dir.IsNearlyZero())
			Dir = FVector::UpVector;
		Pickup->PickupLocationData.ItemOwner = nullptr;
		Pickup->PickupLocationData.PickupTarget = nullptr;
		Pickup->PickupLocationData.CombineTarget = nullptr;
		Pickup->PickupLocationData.LootInitialPosition = *TossFrom;
		Pickup->PickupLocationData.LootFinalPosition = Location;
		Pickup->PickupLocationData.FinalTossRestLocation = Location;
		Pickup->PickupLocationData.FlyTime = TossFlyTime;
		Pickup->PickupLocationData.StartDirection = Dir;
		Pickup->PickupLocationData.TossState = EFortPickupTossState::InProgress;
		Pickup->PickupLocationData.bPlayPickupSound = false;
		Pickup->PickupLocationData.PickupGuid = FGuid::NewGuid();
		FortAthenaLootTransfer::BeginServerToss(Pickup, *TossFrom, Location, TossFlyTime);
	}
	Pickup->SetReplicates(true);
	Pickup->ForceNetUpdate();
	UE_LOG(LogTemp, Display, TEXT("Loot: spawned resolved pickup %s count=%d at (%.0f,%.0f,%.0f) root=%d"), *Definition->GetName(), Count,
		Pickup->GetActorLocation().X, Pickup->GetActorLocation().Y, Pickup->GetActorLocation().Z, Pickup->GetRootComponent() ? 1 : 0);
	return Pickup;
}

namespace FortAthenaLootTransfer
{
	void InstallRpcOverrides();
	bool HandlePickup(AFortPlayerPawn* Pawn, AFortPickup* Pickup,
		float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound);
	void InitializeWorldPickup(AFortPickupAthena* Pickup, const FVector& Location);
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
