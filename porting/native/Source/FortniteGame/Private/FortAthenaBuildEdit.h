// Building edit, replacement, repair, upgrade, and material selection RPCs for the original 12.60 client.
//
// Active only when -FortniteOriginalClient is set (IsEnabled()). Free functions in namespace FortAthenaBuildEdit
// because generated classes may not receive new methods (WORKER_RULES.md). Non-original behavior carries a
// "// stand-in:" comment in the .cpp.
#pragma once

#include "CoreMinimal.h"
#include "EBuildingReplacementType.h"
#include "EFortResourceLevel.h"
#include "EFortResourceType.h"
#include "Templates/SubclassOf.h"

class ABuildingActor;
class ABuildingSMActor;
class AController;
class AFortPlayerController;

namespace FortAthenaBuildEdit
{
	// True when -FortniteOriginalClient is passed on the server command line.
	bool IsEnabled();

	// Rebinds the UFunction::SetNativeFunc pointers on AFortPlayerController for:
	// - ServerBeginEditingBuildingActor
	// - ServerEditBuildingActor
	// - ServerEndEditingBuildingActor
	// - ServerRepairBuildingActor
	// - ServerOnMaterialSelection
	// - ServerUpgradeBuildingActor
	// Idempotent and active only under -FortniteOriginalClient.
	void InstallRpcOverrides();

	// Port of ABuildingSMActor::ReplaceBuildingActor (binary 0x141c19240) + transform (0x141b926c0) +
	// SetMirrored (0x141c1bf20) + health/owner transfer (0x141c0d460).
	ABuildingSMActor* ReplaceBuildingActor(
		ABuildingSMActor* OldActor,
		EBuildingReplacementType ReplacementType,
		TSubclassOf<ABuildingSMActor> NewBuildingClass,
		int32 BuildingLevel,
		int32 RotationIterations,
		bool bMirrored,
		AController* InstigatedBy);

	// Server handler for AFortPlayerController::ServerBeginEditingBuildingActor.
	bool BeginEditingBuildingActor(AFortPlayerController* Controller, ABuildingSMActor* BuildingActorToEdit);

	// Server handler for AFortPlayerController::ServerEditBuildingActor.
	bool EditBuildingActor(
		AFortPlayerController* Controller,
		ABuildingSMActor* BuildingActorToEdit,
		TSubclassOf<ABuildingSMActor> NewBuildingClass,
		uint8 RotationIterations,
		bool bMirrored);

	// Server handler for AFortPlayerController::ServerEndEditingBuildingActor.
	bool EndEditingBuildingActor(AFortPlayerController* Controller, ABuildingSMActor* BuildingActorToStopEditing);

	// Server handler for AFortPlayerController::ServerOnMaterialSelection.
	bool OnMaterialSelection(
		AFortPlayerController* Controller,
		EFortResourceType::Type NewResourceType,
		EFortResourceLevel::Type NewResourceLevel);

	// Server handler for AFortPlayerController::ServerRepairBuildingActor.
	bool RepairBuildingActor(AFortPlayerController* Controller, ABuildingSMActor* BuildingActorToRepair);

	// Server handler for AFortPlayerController::ServerUpgradeBuildingActor (skipped in BR; StW only).
	void UpgradeBuildingActor(AFortPlayerController* Controller, ABuildingActor* BuildingActorToUpgrade, int32 NewUpgradeLevel);
}
