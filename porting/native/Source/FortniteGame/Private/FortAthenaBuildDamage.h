// Player-built piece health, auto-build progression, repair, and damage/destruction for the original 12.60 client.
//
// Active only when -FortniteOriginalClient is set (IsEnabled()). Free functions in namespace FortAthenaBuildDamage
// because generated classes may not receive new methods (WORKER_RULES.md). Re-created from binary analysis of
// 12.60 client functions + cooked curve tables (AthenaAttributesBuildingSection, AttributesBuildingProps,
// AthenaGameData, DefaultGameData); non-original behavior carries a "// stand-in:" comment.
#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

class AActor;
class ABuildingSMActor;
class AController;
class AFortPlayerController;
class UWorld;

namespace FortAthenaBuildDamage
{
	// True when -FortniteOriginalClient is passed on the server command line.
	bool IsEnabled();

	// Initializes a newly placed player-built piece:
	// - Clears bAllowResourceDrop (player builds drop 0 resources in BR)
	// - Sets bPlayerPlaced, OwnerPersistentID (from PlayerState->WorldPlayerId), and team if available
	// - Initializes MaxHealth and initial Health from AthenaAttributesBuildingSection / AttributesBuildingProps
	//   + AthenaGameData BuildingInitialHealthPercent_<Material> (Wood 90/150, Stone 100/300, Metal 110/500)
	// - Populates HealthToAutoBuild / bUnderConstruction and registers for TickAutoBuild progression
	void InitializePlayerBuiltPiece(ABuildingSMActor* Building, AFortPlayerController* OwnerPC);

	// Transfers health fraction and remaining auto-build health from OldBuilding to NewBuilding during
	// ReplaceBuildingActor (original 0x141c0d460 / 0x141c19240 semantics).
	void InitializeEditedBuildingPiece(ABuildingSMActor* OldBuilding, ABuildingSMActor* NewBuilding, AFortPlayerController* OwnerPC);

	// Advances auto-build construction and repair health over BuildTime / RepairTime curves (Wood 4s, Stone 12s, Metal 25s).
	// Call once per server tick from FortniteAthenaGameplay.cpp.
	void TickAutoBuild(UWorld* World, float DeltaSeconds);

	// False for bPlayerPlaced pieces so FortHarvestBuilding::ApplyBuildingDamage suppresses material drops
	// even though FortHarvestFlow sets ABuildingSMActor CDO bAllowResourceDrop = true for level harvestables.
	bool ShouldAllowResourceDrop(const ABuildingSMActor* Building);

	// Computes pickaxe environment damage against a player-built piece via FortHarvest::ComputeMeleeEnvironmentDamage
	// (EnvDmg * SurfaceDamageCategory * SurfaceDamageTeams for Friendly vs Hostile and Wood/Stone/Metal/WeakSpot).
	bool ComputePickaxeDamageForPiece(
		const ABuildingSMActor* Building,
		FName WeaponRow,
		uint8 AttackerTeam,
		bool bWeakSpot,
		float& OutDamage);

	// Applies damage to a player-built piece, suppresses material drops, clears active repair / editing on death,
	// notifies client without resource yield, and triggers destruction when Health <= 0.
	// Call from FortHarvestBuilding::ApplyBuildingDamage when Building->bPlayerPlaced != 0, or direct weapon damage.
	bool OnPlayerBuiltPieceDamaged(
		ABuildingSMActor* Building,
		float Damage,
		AController* Instigator,
		AActor* Causer,
		const FHitResult& Hit,
		bool bWeakSpot);

	// Evaluates repair material cost from DefaultGameData (BuildingCost_<Material> * MissingHealthFraction *
	// BuildingRepairCostMult_<Material>, rounded up, min 1 when damaged). Returns 0 when undamaged or free.
	int32 ComputeRepairCost(const ABuildingSMActor* Building);

	// Validates permissions/range/health, deducts repair cost materials via FortAthenaInventory::RemoveItem,
	// marks bUnderRepair, broadcasts OnRepairStarted / OnRepairBuildingStarted, and schedules RepairTime restoration.
	bool RepairBuildingActor(AFortPlayerController* Controller, ABuildingSMActor* Building);
}
