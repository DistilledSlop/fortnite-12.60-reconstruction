#include "FortAthenaBuildEdit.h"

#include "ABuildingActor.h"
#include "ABuildingSMActor.h"
#include "AFortInventory.h"
#include "AFortPawn.h"
#include "AFortPlayerController.h"
#include "AFortPlayerStateAthena.h"
#include "AFortPlayerStateZone.h"
#include "AFortWeap_EditingTool.h"
#include "AFortWeapon.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "FortAthenaBuildDamage.h"
#include "FortAthenaInventoryFlow.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UFortAssetManager.h"
#include "UFortEditToolItemDefinition.h"
#include "UFortGameData.h"
#include "UFortWorldItem.h"
#include "UObject/Class.h"
#include "UObject/Script.h"
#include "UObject/Stack.h"

DEFINE_LOG_CATEGORY_STATIC(LogFortAthenaBuildEdit, Log, All);

namespace FortAthenaBuildEdit
{
namespace
{
	void BuildEditLogDecision(const FString& Detail)
	{
		UE_LOG(LogFortAthenaBuildEdit, Display, TEXT("Build: %s"), *Detail);
	}

	UFortGameData* BuildEditGetGameDataCommon()
	{
		UFortAssetManager* Manager = GEngine ? Cast<UFortAssetManager>(GEngine->AssetManager) : nullptr;
		return Manager ? Manager->GameDataCommon : nullptr;
	}

	float BuildEditGetMaxDistance(const AFortPlayerController* Controller)
	{
		if (Controller && Controller->EditModeDistance > 0.0f)
		{
			return Controller->EditModeDistance;
		}
		if (const UFortGameData* GameData = BuildEditGetGameDataCommon())
		{
			if (GameData->EditModeCancelDistance > 0.0f)
			{
				return GameData->EditModeCancelDistance;
			}
		}
		return 1500.0f; // stand-in: ~3 grid cells when EditModeDistance/EditModeCancelDistance is unset
	}

	// Confidence: binary (0x14238eb20, 0x1423881c0, 0x141c0f470, 0x141c12d60).
	bool BuildEditCanEditBuilding(const AFortPlayerController* Controller, const ABuildingSMActor* Building, FString& OutReason)
	{
		if (!Controller || !Building || !Building->IsValidLowLevel())
		{
			OutReason = TEXT("null controller or building");
			return false;
		}
		if (Building->bDestroyed)
		{
			OutReason = TEXT("building destroyed");
			return false;
		}
		if (!Building->bPlayerPlaced)
		{
			OutReason = TEXT("building not player-placed");
			return false;
		}

		const AFortPlayerStateAthena* PS = Cast<AFortPlayerStateAthena>(Controller->PlayerState);
		const AFortPlayerStateZone* PSZone = Cast<AFortPlayerStateZone>(Controller->PlayerState);
		if (!PS || !PSZone)
		{
			OutReason = TEXT("missing player state");
			return false;
		}

		if (Building->EditingPlayer != nullptr && Building->EditingPlayer != PSZone)
		{
			OutReason = FString::Printf(TEXT("already edited by %s"), *Building->EditingPlayer->GetName());
			return false;
		}

		const bool bSameTeam = (PS->TeamIndex == Building->TeamIndex);
		const bool bSameOwner = (PS->WorldPlayerId != 0 && PS->WorldPlayerId == Building->OwnerPersistentID);
		if (!bSameTeam && !bSameOwner)
		{
			OutReason = FString::Printf(
				TEXT("permission denied pcTeam=%u bldgTeam=%u pcId=%d bldgOwner=%d"),
				static_cast<uint32>(PS->TeamIndex),
				static_cast<uint32>(Building->TeamIndex),
				static_cast<int32>(PS->WorldPlayerId),
				static_cast<int32>(Building->OwnerPersistentID));
			return false;
		}

		if (const APawn* Pawn = Controller->GetPawn())
		{
			const float MaxDist = BuildEditGetMaxDistance(Controller) * 1.15f;
			const float DistSq = FVector::DistSquared(Pawn->GetActorLocation(), Building->GetActorLocation());
			if (DistSq > MaxDist * MaxDist)
			{
				OutReason = FString::Printf(TEXT("out of range dist=%.1f max=%.1f"), FMath::Sqrt(DistSq), MaxDist);
				return false;
			}
		}

		return true;
	}

	AFortWeap_EditingTool* BuildEditFindEditingToolWeapon(const AFortPawn* Pawn)
	{
		if (!Pawn)
		{
			return nullptr;
		}
		if (AFortWeap_EditingTool* Current = Cast<AFortWeap_EditingTool>(Pawn->CurrentWeapon))
		{
			return Current;
		}
		for (AFortWeapon* Weapon : Pawn->CurrentWeaponList)
		{
			if (AFortWeap_EditingTool* EditTool = Cast<AFortWeap_EditingTool>(Weapon))
			{
				return EditTool;
			}
		}
		return nullptr;
	}

	UFortWorldItem* BuildEditFindEditToolItem(const AFortPlayerController* Controller)
	{
		const AFortInventory* Inventory = Controller ? Controller->WorldInventory : nullptr;
		if (!Inventory)
		{
			return nullptr;
		}
		for (UFortWorldItem* Item : Inventory->Inventory.ItemInstances)
		{
			if (Item && Cast<UFortEditToolItemDefinition>(Item->ItemEntry.ItemDefinition))
			{
				return Item;
			}
		}
		return nullptr;
	}

	// Confidence: binary + source (0x1423922e0, 0x142b47a60 OnRep_EditActor, equips EditTool via FortAthenaInventory::ExecuteInventoryItem).
	AFortWeap_EditingTool* BuildEditUpdateEditingTool(
		AFortPlayerController* Controller,
		ABuildingSMActor* TargetActor,
		bool bConfirmEdit,
		bool bEquipIfNeeded)
	{
		AFortPawn* Pawn = Controller ? Cast<AFortPawn>(Controller->GetPawn()) : nullptr;
		if (!Pawn)
		{
			return nullptr;
		}

		if (bEquipIfNeeded && !Cast<AFortWeap_EditingTool>(Pawn->CurrentWeapon))
		{
			if (UFortWorldItem* EditToolItem = BuildEditFindEditToolItem(Controller))
			{
				FortAthenaInventory::ExecuteInventoryItem(Controller, EditToolItem->ItemEntry.ItemGuid);
			}
		}

		AFortWeap_EditingTool* EditTool = BuildEditFindEditingToolWeapon(Pawn);
		if (EditTool)
		{
			if (!EditTool->GetIsReplicated())
			{
				EditTool->SetReplicates(true);
			}
			EditTool->EditActor = TargetActor;
			EditTool->bEditConfirmed = bConfirmEdit ? true : false;
			EditTool->OnRep_EditActor();
			EditTool->ForceNetUpdate();
		}
		return EditTool;
	}

	// Confidence: binary (0x141b926c0 Building_ComputeEditedTransform).
	FTransform BuildEditComputeEditedTransform(const ABuildingSMActor* OldActor, int32 RotationIterations)
	{
		const FTransform OldTransform = OldActor->GetTransform();
		const FRotator OldRot = OldActor->GetActorRotation();
		FRotator NewRot = OldRot;
		const int32 NormIter = ((RotationIterations % 4) + 4) % 4;
		NewRot.Yaw = FRotator::NormalizeAxis(OldRot.Yaw + static_cast<float>(NormIter) * 90.0f);

		FVector NewLoc = OldActor->GetActorLocation();
		if (!OldActor->bRotateInPlaceGame && NormIter != 0)
		{
			const FVector Pivot = OldActor->BaseLocToPivotOffset;
			NewLoc = NewLoc + OldRot.RotateVector(Pivot) - NewRot.RotateVector(Pivot);
		}

		return FTransform(NewRot, NewLoc, OldTransform.GetScale3D());
	}

	// Confidence: binary (0x141c1bf20 ABuildingSMActor::SetMirrored — ungenerated, implemented directly).
	void BuildEditApplyMirrored(ABuildingSMActor* Actor, bool bMirrored)
	{
		if (!Actor)
		{
			return;
		}
		Actor->bMirrored = bMirrored ? 1 : 0;
		if (UStaticMeshComponent* Mesh = Actor->StaticMeshComponent)
		{
			FVector Scale = Mesh->GetRelativeScale3D();
			const float TargetX = bMirrored ? -FMath::Abs(Scale.X) : FMath::Abs(Scale.X);
			if (!FMath::IsNearlyEqual(Scale.X, TargetX))
			{
				Scale.X = TargetX;
				Mesh->SetRelativeScale3D(Scale);
			}
		}
	}

	void BuildEditClearEditingState(AFortPlayerController* Controller, ABuildingSMActor* Building, bool bConfirmEdit)
	{
		AFortPlayerStateZone* PSZone = Controller ? Cast<AFortPlayerStateZone>(Controller->PlayerState) : nullptr;
		if (Building && Building->IsValidLowLevel())
		{
			if (Building->EditingPlayer == nullptr || Building->EditingPlayer == PSZone)
			{
				Building->EditingPlayer = nullptr;
				Building->bCurrentlyBeingEdited = 0;
				Building->OnRep_EditingPlayer();
				Building->ForceNetUpdate();
			}
		}

		if (Controller && Controller->EditBuildingActor == Building)
		{
			Controller->EditBuildingActor = nullptr;
		}

		BuildEditUpdateEditingTool(Controller, nullptr, bConfirmEdit, false);
	}

	// --- Native RPC thunks matching AFortPlayerController.gen.cpp stack layout ---

	static void BuildEditExecServerBeginEditingBuildingActor(UObject* Context, FFrame& Stack, RESULT_DECL)
	{
		P_GET_OBJECT(ABuildingSMActor, BuildingActorToEdit);
		P_FINISH;
		P_NATIVE_BEGIN;
		if (AFortPlayerController* Controller = Cast<AFortPlayerController>(Context))
		{
			BeginEditingBuildingActor(Controller, BuildingActorToEdit);
		}
		P_NATIVE_END;
	}

	static void BuildEditExecServerEditBuildingActor(UObject* Context, FFrame& Stack, RESULT_DECL)
	{
		P_GET_OBJECT(ABuildingSMActor, BuildingActorToEdit);
		P_GET_OBJECT(UClass, NewBuildingClass);
		P_GET_PROPERTY(FByteProperty, RotationIterations);
		P_GET_UBOOL(bMirrored);
		P_FINISH;
		P_NATIVE_BEGIN;
		if (AFortPlayerController* Controller = Cast<AFortPlayerController>(Context))
		{
			EditBuildingActor(
				Controller,
				BuildingActorToEdit,
				TSubclassOf<ABuildingSMActor>(NewBuildingClass),
				RotationIterations,
				bMirrored != 0);
		}
		P_NATIVE_END;
	}

	static void BuildEditExecServerEndEditingBuildingActor(UObject* Context, FFrame& Stack, RESULT_DECL)
	{
		P_GET_OBJECT(ABuildingSMActor, BuildingActorToStopEditing);
		P_FINISH;
		P_NATIVE_BEGIN;
		if (AFortPlayerController* Controller = Cast<AFortPlayerController>(Context))
		{
			EndEditingBuildingActor(Controller, BuildingActorToStopEditing);
		}
		P_NATIVE_END;
	}

	static void BuildEditExecServerRepairBuildingActor(UObject* Context, FFrame& Stack, RESULT_DECL)
	{
		P_GET_OBJECT(ABuildingSMActor, BuildingActorToRepair);
		P_FINISH;
		P_NATIVE_BEGIN;
		if (AFortPlayerController* Controller = Cast<AFortPlayerController>(Context))
		{
			RepairBuildingActor(Controller, BuildingActorToRepair);
		}
		P_NATIVE_END;
	}

	static void BuildEditExecServerOnMaterialSelection(UObject* Context, FFrame& Stack, RESULT_DECL)
	{
		P_GET_PROPERTY(FByteProperty, NewResourceType);
		P_GET_PROPERTY(FByteProperty, NewResourceLevel);
		P_FINISH;
		P_NATIVE_BEGIN;
		if (AFortPlayerController* Controller = Cast<AFortPlayerController>(Context))
		{
			OnMaterialSelection(
				Controller,
				static_cast<EFortResourceType::Type>(NewResourceType),
				static_cast<EFortResourceLevel::Type>(NewResourceLevel));
		}
		P_NATIVE_END;
	}

	static void BuildEditExecServerUpgradeBuildingActor(UObject* Context, FFrame& Stack, RESULT_DECL)
	{
		P_GET_OBJECT(ABuildingActor, BuildingActorToUpgrade);
		P_GET_PROPERTY(FIntProperty, NewUpgradeLevel);
		P_FINISH;
		P_NATIVE_BEGIN;
		if (AFortPlayerController* Controller = Cast<AFortPlayerController>(Context))
		{
			UpgradeBuildingActor(Controller, BuildingActorToUpgrade, NewUpgradeLevel);
		}
		P_NATIVE_END;
	}

	void BuildEditBindControllerRpc(const TCHAR* Name, FNativeFuncPtr Func)
	{
		UFunction* Fn = AFortPlayerController::StaticClass()->FindFunctionByName(FName(Name));
		if (!Fn)
		{
			UE_LOG(LogFortAthenaBuildEdit, Warning, TEXT("Build: AFortPlayerController::%s UFunction not found"), Name);
			return;
		}
		Fn->SetNativeFunc(Func);
		BuildEditLogDecision(FString::Printf(TEXT("Installed RPC override AFortPlayerController::%s"), Name));
	}

	bool BuildEditRpcOverridesInstalled = false;
} // namespace

bool IsEnabled()
{
	return FParse::Param(FCommandLine::Get(), TEXT("FortniteOriginalClient"));
}

// stand-in: runtime UFunction::SetNativeFunc rebinding until bodies are promoted through porting/behavior/*.inc.
void InstallRpcOverrides()
{
	if (!IsEnabled() || BuildEditRpcOverridesInstalled)
	{
		return;
	}
	BuildEditRpcOverridesInstalled = true;

	BuildEditBindControllerRpc(TEXT("ServerBeginEditingBuildingActor"), reinterpret_cast<FNativeFuncPtr>(&BuildEditExecServerBeginEditingBuildingActor));
	BuildEditBindControllerRpc(TEXT("ServerEditBuildingActor"), reinterpret_cast<FNativeFuncPtr>(&BuildEditExecServerEditBuildingActor));
	BuildEditBindControllerRpc(TEXT("ServerEndEditingBuildingActor"), reinterpret_cast<FNativeFuncPtr>(&BuildEditExecServerEndEditingBuildingActor));
	BuildEditBindControllerRpc(TEXT("ServerRepairBuildingActor"), reinterpret_cast<FNativeFuncPtr>(&BuildEditExecServerRepairBuildingActor));
	BuildEditBindControllerRpc(TEXT("ServerOnMaterialSelection"), reinterpret_cast<FNativeFuncPtr>(&BuildEditExecServerOnMaterialSelection));
	BuildEditBindControllerRpc(TEXT("ServerUpgradeBuildingActor"), reinterpret_cast<FNativeFuncPtr>(&BuildEditExecServerUpgradeBuildingActor));
}

// Confidence: binary (0x141c19240 ReplaceBuildingActor, 0x141b926c0, 0x141c1bf20, 0x141c0d460).
ABuildingSMActor* ReplaceBuildingActor(
	ABuildingSMActor* OldActor,
	EBuildingReplacementType ReplacementType,
	TSubclassOf<ABuildingSMActor> NewBuildingClass,
	int32 BuildingLevel,
	int32 RotationIterations,
	bool bMirrored,
	AController* InstigatedBy)
{
	if (!IsEnabled() || !OldActor || !OldActor->IsValidLowLevel() || OldActor->bDestroyed)
	{
		BuildEditLogDecision(TEXT("ReplaceBuildingActor aborted: invalid or destroyed OldActor"));
		return nullptr;
	}

	UClass* SpawnClass = NewBuildingClass.Get();
	if (!SpawnClass || !SpawnClass->IsChildOf(ABuildingSMActor::StaticClass()) || SpawnClass->HasAnyClassFlags(CLASS_Abstract))
	{
		BuildEditLogDecision(FString::Printf(
			TEXT("ReplaceBuildingActor aborted: invalid NewBuildingClass=%s"),
			SpawnClass ? *SpawnClass->GetName() : TEXT("null")));
		return nullptr;
	}

	UWorld* World = OldActor->GetWorld();
	if (!World)
	{
		BuildEditLogDecision(TEXT("ReplaceBuildingActor aborted: null world"));
		return nullptr;
	}

	const int32 NormRot = ((RotationIterations % 4) + 4) % 4;
	const FTransform NewTransform = BuildEditComputeEditedTransform(OldActor, NormRot);
	APawn* InstigatorPawn = InstigatedBy ? InstigatedBy->GetPawn() : OldActor->GetInstigator();

	ABuildingSMActor* NewActor = World->SpawnActorDeferred<ABuildingSMActor>(
		SpawnClass,
		NewTransform,
		OldActor->GetOwner(),
		InstigatorPawn,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (!NewActor)
	{
		BuildEditLogDecision(FString::Printf(TEXT("ReplaceBuildingActor aborted: SpawnActorDeferred failed for %s"), *SpawnClass->GetName()));
		return nullptr;
	}

	if (!NewActor->GetIsReplicated())
	{
		NewActor->SetReplicates(true);
	}

	NewActor->CurrentBuildingLevel = BuildingLevel;
	NewActor->bPlayerPlaced = OldActor->bPlayerPlaced;
	NewActor->bAllowResourceDrop = OldActor->bPlayerPlaced ? 0 : OldActor->bAllowResourceDrop;
	NewActor->OwnerPersistentID = OldActor->OwnerPersistentID;
	NewActor->SetTeam(OldActor->TeamIndex);
	BuildEditApplyMirrored(NewActor, bMirrored);

	for (ABuildingSMActor* Attached : OldActor->AttachedBuildingActors)
	{
		if (Attached && Attached->IsValidLowLevel() && !Attached->bDestroyed)
		{
			NewActor->AttachedBuildingActors.Add(Attached);
		}
	}

	NewActor->FinishSpawning(NewTransform);

	// Initialize NewActor health proportional to OldActor health & auto-build state after components/ASC exist.
	FortAthenaBuildDamage::InitializeEditedBuildingPiece(OldActor, NewActor, Cast<AFortPlayerController>(InstigatedBy));

	const FString OldName = OldActor->GetName();
	OldActor->OnReplacementDestruction.Broadcast(ReplacementType, NewActor);
	OldActor->EditingPlayer = nullptr;
	OldActor->bCurrentlyBeingEdited = 0;
	OldActor->SetActorEnableCollision(false);
	OldActor->bDestroyed = 1;
	OldActor->HandleDestroyed();
	OldActor->Destroy();

	NewActor->ForceNetUpdate();

	BuildEditLogDecision(FString::Printf(
		TEXT("ReplaceBuildingActor %s -> %s rot=%d mirrored=%d team=%u level=%d loc=%s"),
		*OldName,
		*NewActor->GetName(),
		NormRot,
		bMirrored ? 1 : 0,
		static_cast<uint32>(NewActor->TeamIndex),
		BuildingLevel,
		*NewTransform.GetLocation().ToString()));
	return NewActor;
}

// Confidence: binary + inferred (0x142a4a290, 0x1423716f0, 0x142b47a60, 0x141c12d60).
bool BeginEditingBuildingActor(AFortPlayerController* Controller, ABuildingSMActor* BuildingActorToEdit)
{
	if (!IsEnabled() || !Controller)
	{
		return false;
	}

	FString RejectReason;
	if (!BuildEditCanEditBuilding(Controller, BuildingActorToEdit, RejectReason))
	{
		BuildEditLogDecision(FString::Printf(
			TEXT("ServerBeginEditingBuildingActor refused pc=%s target=%s reason=%s"),
			*Controller->GetName(),
			BuildingActorToEdit ? *BuildingActorToEdit->GetName() : TEXT("null"),
			*RejectReason));
		Controller->ClientFailedToBeginEditingBuildingActor(BuildingActorToEdit);
		return false;
	}

	if (Controller->EditBuildingActor && Controller->EditBuildingActor != BuildingActorToEdit)
	{
		BuildEditClearEditingState(Controller, Controller->EditBuildingActor, false);
	}

	AFortPlayerStateZone* PSZone = Cast<AFortPlayerStateZone>(Controller->PlayerState);
	BuildingActorToEdit->EditingPlayer = PSZone;
	BuildingActorToEdit->bCurrentlyBeingEdited = 1;
	BuildingActorToEdit->OnRep_EditingPlayer();
	BuildingActorToEdit->ForceNetUpdate();

	Controller->EditBuildingActor = BuildingActorToEdit;

	AFortWeap_EditingTool* EditTool = BuildEditUpdateEditingTool(Controller, BuildingActorToEdit, false, true);

	BuildEditLogDecision(FString::Printf(
		TEXT("ServerBeginEditingBuildingActor accepted pc=%s target=%s editTool=%s"),
		*Controller->GetName(),
		*BuildingActorToEdit->GetName(),
		EditTool ? *EditTool->GetName() : TEXT("none")));
	return true;
}

// Confidence: binary + inferred (0x142a4b0b0, 0x141c12f90, 0x141c19240).
bool EditBuildingActor(
	AFortPlayerController* Controller,
	ABuildingSMActor* BuildingActorToEdit,
	TSubclassOf<ABuildingSMActor> NewBuildingClass,
	uint8 RotationIterations,
	bool bMirrored)
{
	if (!IsEnabled() || !Controller)
	{
		return false;
	}

	if (!BuildingActorToEdit || !BuildingActorToEdit->IsValidLowLevel() || BuildingActorToEdit->bDestroyed)
	{
		BuildEditLogDecision(FString::Printf(TEXT("ServerEditBuildingActor refused pc=%s: invalid/destroyed target"), *Controller->GetName()));
		BuildEditClearEditingState(Controller, BuildingActorToEdit, false);
		return false;
	}

	UClass* NewClass = NewBuildingClass.Get();
	if (!NewClass || !NewClass->IsChildOf(ABuildingSMActor::StaticClass()) || NewClass->HasAnyClassFlags(CLASS_Abstract))
	{
		BuildEditLogDecision(FString::Printf(
			TEXT("ServerEditBuildingActor refused pc=%s target=%s: invalid NewBuildingClass=%s"),
			*Controller->GetName(),
			*BuildingActorToEdit->GetName(),
			NewClass ? *NewClass->GetName() : TEXT("null")));
		BuildEditClearEditingState(Controller, BuildingActorToEdit, false);
		return false;
	}

	const AFortPlayerStateZone* PSZone = Cast<AFortPlayerStateZone>(Controller->PlayerState);
	if (!PSZone || BuildingActorToEdit->EditingPlayer != PSZone)
	{
		BuildEditLogDecision(FString::Printf(
			TEXT("ServerEditBuildingActor refused pc=%s target=%s: EditingPlayer=%s mismatch"),
			*Controller->GetName(),
			*BuildingActorToEdit->GetName(),
			BuildingActorToEdit->EditingPlayer ? *BuildingActorToEdit->EditingPlayer->GetName() : TEXT("null")));
		if (Controller->EditBuildingActor == BuildingActorToEdit)
		{
			Controller->EditBuildingActor = nullptr;
			BuildEditUpdateEditingTool(Controller, nullptr, false, false);
		}
		return false;
	}

	FString RejectReason;
	if (!BuildEditCanEditBuilding(Controller, BuildingActorToEdit, RejectReason))
	{
		BuildEditLogDecision(FString::Printf(
			TEXT("ServerEditBuildingActor refused pc=%s target=%s reason=%s"),
			*Controller->GetName(),
			*BuildingActorToEdit->GetName(),
			*RejectReason));
		BuildEditClearEditingState(Controller, BuildingActorToEdit, false);
		return false;
	}

	// stand-in: validate class lineage against bIsPlayerBuildable or Player blueprint path; full EMP compatibility
	// pattern validation (0x141c080a0) is handled client-side by UBuildingEditModeSupport.
	const ABuildingSMActor* NewDefault = Cast<ABuildingSMActor>(NewClass->GetDefaultObject());
	const bool bPlayerBuildableClass = NewDefault &&
		(NewDefault->bIsPlayerBuildable || NewClass->GetPathName().StartsWith(TEXT("/Game/Building/ActorBlueprints/Player/")));
	if (!bPlayerBuildableClass)
	{
		BuildEditLogDecision(FString::Printf(
			TEXT("ServerEditBuildingActor refused pc=%s: class %s is not player-buildable"),
			*Controller->GetName(),
			*NewClass->GetName()));
		BuildEditClearEditingState(Controller, BuildingActorToEdit, false);
		return false;
	}

	BuildEditClearEditingState(Controller, BuildingActorToEdit, true);

	const int32 NormRot = static_cast<int32>(RotationIterations & 3);
	ABuildingSMActor* NewActor = ReplaceBuildingActor(
		BuildingActorToEdit,
		EBuildingReplacementType::BRT_Edited,
		NewBuildingClass,
		BuildingActorToEdit->GetCurrentBuildingLevel(),
		NormRot,
		bMirrored,
		Controller);

	return NewActor != nullptr;
}

// Confidence: binary + inferred (0x142a4b300, 0x142397520).
bool EndEditingBuildingActor(AFortPlayerController* Controller, ABuildingSMActor* BuildingActorToStopEditing)
{
	if (!IsEnabled() || !Controller)
	{
		return false;
	}

	ABuildingSMActor* Target = BuildingActorToStopEditing ? BuildingActorToStopEditing : Controller->EditBuildingActor;
	BuildEditClearEditingState(Controller, Target, false);

	BuildEditLogDecision(FString::Printf(
		TEXT("ServerEndEditingBuildingActor pc=%s target=%s"),
		*Controller->GetName(),
		Target ? *Target->GetName() : TEXT("none")));
	return true;
}

// Confidence: binary + D5 (0x142a4c000).
bool OnMaterialSelection(
	AFortPlayerController* Controller,
	EFortResourceType::Type NewResourceType,
	EFortResourceLevel::Type NewResourceLevel)
{
	if (!IsEnabled() || !Controller)
	{
		return false;
	}

	if (NewResourceType >= EFortResourceType::None || NewResourceLevel >= EFortResourceLevel::NumLevels)
	{
		BuildEditLogDecision(FString::Printf(
			TEXT("ServerOnMaterialSelection refused pc=%s invalid type=%d level=%d"),
			*Controller->GetName(),
			static_cast<int32>(NewResourceType),
			static_cast<int32>(NewResourceLevel)));
		return false;
	}

	Controller->CurrentResourceType = NewResourceType;
	Controller->CurrentResourceLevel = NewResourceLevel;

	BuildEditLogDecision(FString::Printf(
		TEXT("ServerOnMaterialSelection pc=%s type=%d level=%d"),
		*Controller->GetName(),
		static_cast<int32>(NewResourceType),
		static_cast<int32>(NewResourceLevel)));
	return true;
}

// Confidence: binary (0x142a4c840 -> 0x141c18db0).
bool RepairBuildingActor(AFortPlayerController* Controller, ABuildingSMActor* BuildingActorToRepair)
{
	if (!IsEnabled())
	{
		return false;
	}
	return FortAthenaBuildDamage::RepairBuildingActor(Controller, BuildingActorToRepair);
}

// Confidence: binary + D5 (0x142a4e9c0 — StW only, no-op in BR).
void UpgradeBuildingActor(AFortPlayerController* Controller, ABuildingActor* BuildingActorToUpgrade, int32 NewUpgradeLevel)
{
	if (!IsEnabled())
	{
		return;
	}
	BuildEditLogDecision(FString::Printf(
		TEXT("ServerUpgradeBuildingActor skipped (BR mode) pc=%s target=%s level=%d"),
		Controller ? *Controller->GetName() : TEXT("null"),
		BuildingActorToUpgrade ? *BuildingActorToUpgrade->GetName() : TEXT("null"),
		NewUpgradeLevel));
}
} // namespace FortAthenaBuildEdit
