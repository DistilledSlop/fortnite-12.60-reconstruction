#pragma once

#include "CoreMinimal.h"
#include "EFortPickupSpawnSource.h"
#include "EFortQuickBars.h"

class AFortPawn;
class AFortPickup;
class AFortPickupAthena;
class AFortPlayerController;
class AFortPlayerPawn;
class UFortWorldItem;
class UWorld;
struct FFortItemEntry;

namespace FortAthenaLootTransfer
{
	// Gated behind -FortniteOriginalClient to match FortAthenaLoot / FortAthenaInventoryFlow.
	bool IsEnabled();

	// Rebinds stripped shipping RPC thunks for pickups, swaps, inventory drop, and quickbar acks:
	// - AFortPlayerPawn::ServerHandlePickup (exec 0x142a7ed80)
	// - AFortPlayerPawn::ServerHandlePickupWithRequestedSwap (exec 0x142a7ef10)
	// - AFortPlayerPawn::ServerHandlePickupWithSwap (exec 0x142a7f100)
	// - AFortPlayerController::ServerAttemptInventoryDrop (exec 0x142a4a130)
	// - AFortPlayerController::ServerAcknowledgeDelayedQuickBarAction (exec 0x142a3e490 / stub 0x1406801c0)
	void InstallRpcOverrides();

	// Scans Controller->WorldInventory and sends DelayedQuickBarActions for any quickbar-eligible
	// items (pickaxe, weapons, consumables, building pieces, traps) not yet tracked on this server.
	// Safe and idempotent to call after SetupPlayer and on each pickup/drop.
	void SetupPlayerQuickBars(AFortPlayerController* Controller);

	// Assigns a quickbar slot (or honors PreferredBar/PreferredSlot) and replicates Add via
	// AFortPlayerController::DelayedQuickBarActions (0x1423684c0 / 0x1423679c0).
	bool OnItemGranted(AFortPlayerController* Controller, UFortWorldItem* Item,
		EFortQuickBars PreferredBar = EFortQuickBars::Max_None,
		int32 PreferredSlot = INDEX_NONE,
		bool bActivateSlot = false);

	// Removes ItemGuid from server quickbar tracking and replicates Remove via DelayedQuickBarActions
	// (0x1423a83a0 / 0x1423684c0).
	void OnItemRemoved(AFortPlayerController* Controller, const FGuid& ItemGuid);

	// Full pickup lifecycle handlers replacing the limited immediate-collection stand-in.
	bool HandlePickup(AFortPlayerPawn* Pawn, AFortPickup* Pickup,
		float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound);

	bool HandlePickupWithSwap(AFortPlayerPawn* Pawn, AFortPickup* Pickup,
		const FGuid& SwapGuid, float InFlyTime, const FVector& InStartDirection,
		bool bPlayPickupSound, bool bRequestedSwap);

	// Reconstructs AFortPlayerController::ServerAttemptInventoryDrop (exec 0x142a4a130).
	bool AttemptInventoryDrop(AFortPlayerController* Controller, const FGuid& ItemGuid,
		int32 Count, bool bTrash);

	// Prunes client-acknowledged action IDs from Controller->DelayedQuickBarActions.Items.
	void AcknowledgeDelayedQuickBarActions(AFortPlayerController* Controller,
		const TArray<uint32>& ProcessedActionIds);

	// Spawns an outgoing AFortPickupAthena initialized with TossPickup location/interpolation data
	// (0x14203cda0, 0x142029370, 0x14203b400, 0x142039830) and ctor defaults (0x1420231b0).
	AFortPickupAthena* SpawnTossedPickupFromEntry(UWorld* World, const FFortItemEntry& Entry,
		const FVector& StartLocation, const FVector& TossDirection, AFortPawn* OwnerPawn,
		uint8 SourceTypeFlags, EFortPickupSpawnSource SpawnSource, float FlyTime = -1.0f);
}
