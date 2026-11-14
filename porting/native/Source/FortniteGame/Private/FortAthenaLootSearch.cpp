#include "FortAthenaLootSearch.h"

#include "ABuildingContainer.h"
#include "AFortGameModeAthena.h"
#include "AFortPawn.h"
#include "AFortPickupAthena.h"
#include "AFortPlayerController.h"
#include "AFortPlayerPawn.h"
#include "Components/StaticMeshComponent.h"
#include "EInteractionBeingAttempted.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "FFortBaseWeaponStats.h"
#include "FFortLootLevelData.h"
#include "FortAthenaLoot.h"
#include "FortAthenaLootData.h"
#include "FortAthenaLootRoll.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInterface.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TInteractionType.h"
#include "UFortControllerComponent_Interaction.h"
#include "UFortItemDefinition.h"
#include "UFortWeaponItemDefinition.h"
#include "UFortWorldItemDefinition.h"
#include "UObject/ScriptMacros.h"

namespace
{
	// stand-in: 0x141d08ed0 reads URuntimeOptions+0x690 or InteractionComponent+0x168 (not in generated reflection);
	// 500 cm matches standard BR search range before the 2x multiplier in 0x141d14c20.
	constexpr float DefaultInteractValidationDistance = 500.0f;

	double SampleUnitRandom()
	{
		return static_cast<double>(FMath::RandRange(0, 999999)) / 1000000.0;
	}

	bool IsInteractableContainer(const ABuildingContainer* Container, FString& OutReason)
	{
		if (!IsValid(Container) || Container->IsPendingKill())
		{
			OutReason = TEXT("invalid_or_pending_kill");
			return false;
		}
		if (Container->bDestroyed)
		{
			OutReason = TEXT("already_destroyed");
			return false;
		}
		if (!Container->bAllowInteract)
		{
			OutReason = TEXT("interact_disabled");
			return false;
		}
		if (Container->bAlreadySearched)
		{
			OutReason = TEXT("already_searched");
			return false;
		}
		return true;
	}

	// Mirrors 0x141d14c20:157-172 bounding-box distance validation around ReceivingActor bounds.
	bool PassesDistanceCheck(const AFortPawn* Pawn, const ABuildingContainer* Container, float& OutDist2D, float& OutDistZ)
	{
		if (!Pawn || !Container)
			return false;

		FVector BoundsOrigin = FVector::ZeroVector;
		FVector BoundsExtent = FVector::ZeroVector;
		Container->GetActorBounds(true, BoundsOrigin, BoundsExtent);
		const FVector PawnLocation = Pawn->GetActorLocation();

		OutDist2D = FVector::Dist2D(BoundsOrigin, PawnLocation);
		OutDistZ = FMath::Abs(BoundsOrigin.Z - PawnLocation.Z);

		const float Extent2D = FMath::Sqrt(BoundsExtent.X * BoundsExtent.X + BoundsExtent.Y * BoundsExtent.Y);
		const float MaxDist2D = (2.0f * DefaultInteractValidationDistance) + Extent2D;
		const float MaxDistZ = (2.0f * DefaultInteractValidationDistance) + BoundsExtent.Z;
		return OutDist2D <= MaxDist2D && OutDistZ <= MaxDistZ;
	}

	// Computes drop world spawn point around WorldOrigin.
	// stand-in: radial offset along Container's toss yaw until AFortPickup toss flight (PickupLocationData) is ported.
	FVector ComputeDropSpawnLocation(
		const ABuildingContainer* Container,
		const FVector& WorldOrigin,
		int32 DropIndex,
		int32 TotalDrops,
		float ScatterRadius)
	{
		if (TotalDrops <= 1 || ScatterRadius <= 0.0f)
			return WorldOrigin;

		const float BaseYawDeg = Container
			? (Container->GetActorRotation().Yaw + Container->LootTossDirection_Athena.Yaw)
			: 0.0f;
		const float HalfConeDeg = (Container && Container->LootTossConeHalfAngle_Athena > 5.0f)
			? FMath::Min(Container->LootTossConeHalfAngle_Athena, 180.0f)
			: 60.0f;
		const float StepFraction = (TotalDrops > 1)
			? (static_cast<float>(DropIndex) / static_cast<float>(TotalDrops - 1)) - 0.5f
			: 0.0f;
		const float AngleRad = FMath::DegreesToRadians(BaseYawDeg + StepFraction * 2.0f * HalfConeDeg);
		const float Radius = FMath::Clamp(ScatterRadius * FMath::FRandRange(0.55f, 1.0f), 15.0f, 120.0f);
		return WorldOrigin + FVector(FMath::Cos(AngleRad) * Radius, FMath::Sin(AngleRad) * Radius, 6.0f);
	}

	// Commits searched visuals and COND_Custom/Destroy replication matching 0x141be85b0 + 0x141be9dd0.
	void CommitContainerSearched(ABuildingContainer* Container, AFortPawn* InteractingPawn)
	{
		if (!IsValid(Container))
			return;

		Container->bAlreadySearched = 1;
		if (InteractingPawn)
			Container->OnInteract.Broadcast(Container, InteractingPawn);

		if (Container->bDestroyContainerOnSearch && !Container->bAlwaysShowContainer)
		{
			Container->ForceNetUpdate();
			UE_LOG(LogTemp, Display, TEXT("LootSearch: destroying %s after search (bDestroyContainerOnSearch=1)"), *Container->GetName());
			Container->Destroy();
			return;
		}

		if (UStaticMeshComponent* MeshComp = Container->StaticMeshComponent)
		{
			if (Container->SearchedMesh)
				MeshComp->SetStaticMesh(Container->SearchedMesh);
			for (int32 MatIdx = 0; MatIdx < Container->SearchedMaterialOverrides.Num(); ++MatIdx)
			{
				if (UMaterialInterface* Mat = Container->SearchedMaterialOverrides[MatIdx])
					MeshComp->SetMaterial(MatIdx, Mat);
			}
		}

		Container->OnSetSearched();
		Container->OnRep_bAlreadySearched();
		Container->ForceNetUpdate();
		UE_LOG(LogTemp, Display, TEXT("LootSearch: marked %s bAlreadySearched=1 (note: generated ABuildingContainer lacks PreReplication for COND_Custom)"), *Container->GetName());
	}

	// Custom native handler for UFortControllerComponent_Interaction::ServerAttemptInteract (0x1428a5d20).
	// Reads exact UHT parameters from Stack, handles ABuildingContainer when enabled, and preserves
	// ServerAttemptInteract_Implementation for all other interactables without double-consuming Stack.
	void ExecServerAttemptInteract(UObject* Context, FFrame& Stack, RESULT_DECL)
	{
		P_GET_OBJECT(AActor, ReceivingActor);
		P_GET_OBJECT(UPrimitiveComponent, InteractComponent);
		P_GET_PROPERTY(FByteProperty, RawInteractType);
		P_GET_OBJECT(UObject, OptionalObjectData);
		P_GET_PROPERTY(FByteProperty, RawInteractionBeingAttempted);
		P_GET_PROPERTY(FIntProperty, RequestId);
		P_FINISH;

		const TInteractionType InteractType = static_cast<TInteractionType>(RawInteractType);
		const EInteractionBeingAttempted InteractionBeingAttempted = static_cast<EInteractionBeingAttempted>(RawInteractionBeingAttempted);

		P_NATIVE_BEGIN;
		UFortControllerComponent_Interaction* Component = Cast<UFortControllerComponent_Interaction>(Context);
		if (!Component)
			return;

		ABuildingContainer* Container = Cast<ABuildingContainer>(ReceivingActor);
		if (!FortAthenaLootSearch::IsEnabled() || !Container)
		{
			if (FortAthenaLootSearch::IsEnabled())
			{
				UE_LOG(LogTemp, Display, TEXT("LootSearch: non-container interactable=%s type=%d -> delegating to ServerAttemptInteract_Implementation (req=%d)"),
					*GetNameSafe(ReceivingActor), static_cast<int32>(InteractType), RequestId);
			}
			// Generated UFortControllerComponent_Interaction::ServerAttemptInteract_Implementation is currently a stub;
			// calling it here preserves any future promoted implementation cleanly.
			Component->ServerAttemptInteract_Implementation(
				ReceivingActor, InteractComponent, InteractType, OptionalObjectData, InteractionBeingAttempted, RequestId);
			return;
		}

		AFortPlayerController* Controller = Cast<AFortPlayerController>(Component->GetOwner());
		if (!Controller || !Controller->HasAuthority())
		{
			UE_LOG(LogTemp, Warning, TEXT("LootSearch: rejected container=%s (missing authority controller=%s)"),
				*Container->GetName(), *GetNameSafe(Component->GetOwner()));
			return;
		}

		if (InteractType != TInteractionType::IT_Simple && InteractType != TInteractionType::IT_LongPress)
		{
			UE_LOG(LogTemp, Display, TEXT("LootSearch: rejected container=%s player=%s invalid interact type=%d"),
				*Container->GetName(), *Controller->GetName(), static_cast<int32>(InteractType));
			return;
		}

		UE_LOG(LogTemp, Display, TEXT("LootSearch: attempt container=%s player=%s type=%d attempt=%d req=%d"),
			*Container->GetName(), *Controller->GetName(), static_cast<int32>(InteractType),
			static_cast<int32>(InteractionBeingAttempted), RequestId);

		// Note: 0x141d15810 does not forward RequestId into 0x141d14c20 and no client acknowledgement RPC
		// carrying RequestId is sent by 0x141d14c20.
		FortAthenaLootSearch::TrySearchContainer(Controller, Container);
		P_NATIVE_END;
	}
}

bool FortAthenaLootSearch::IsEnabled()
{
	return FParse::Param(FCommandLine::Get(), TEXT("FortniteOriginalClient"));
}

void FortAthenaLootSearch::InstallRpcOverrides()
{
	if (!IsEnabled())
		return;

	static bool bInstalled = false;
	if (bInstalled)
		return;

	if (UFunction* Function = UFortControllerComponent_Interaction::StaticClass()->FindFunctionByName(TEXT("ServerAttemptInteract")))
	{
		Function->SetNativeFunc(&ExecServerAttemptInteract);
		bInstalled = true;
		UE_LOG(LogTemp, Display, TEXT("LootSearch: bound UFortControllerComponent_Interaction::ServerAttemptInteract (0x1428a5d20 / 0x141d14c20)"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("LootSearch: UFortControllerComponent_Interaction::ServerAttemptInteract UFunction missing"));
	}
}

bool FortAthenaLootSearch::ResolveItemLevel(
	const UDataTable* LevelTable,
	UFortItemDefinition* Definition,
	int32 LootLevel,
	double Random01,
	int32& OutLevel)
{
	int32 ItemMin = 0;
	int32 ItemMax = 0;
	FName Category(TEXT("Default"));

	if (const UFortWorldItemDefinition* WorldDef = Cast<UFortWorldItemDefinition>(Definition))
	{
		ItemMin = WorldDef->MinLevel;
		ItemMax = WorldDef->MaxLevel;
		if (WorldDef->LootLevelData.RowContents != NAME_None)
			Category = WorldDef->LootLevelData.RowContents;
		if (WorldDef->LootLevelData.DataTable)
			LevelTable = WorldDef->LootLevelData.DataTable;
	}

	const FFortLootLevelData* BestExact = nullptr;
	const FFortLootLevelData* BestFloor = nullptr;
	if (LevelTable && LevelTable->GetRowStruct() && LevelTable->GetRowStruct()->IsChildOf(FFortLootLevelData::StaticStruct()))
	{
		LevelTable->ForeachRow<FFortLootLevelData>(TEXT("FortAthenaLootSearch::ResolveItemLevel"),
			[Category, LootLevel, &BestExact, &BestFloor](const FName&, const FFortLootLevelData& Row)
			{
				if (Row.Category != Category)
					return;
				if (Row.LootLevel == LootLevel)
					BestExact = &Row;
				else if (Row.LootLevel <= LootLevel && (!BestFloor || Row.LootLevel > BestFloor->LootLevel))
					BestFloor = &Row;
			});
	}

	const FFortLootLevelData* Chosen = BestExact ? BestExact : BestFloor;
	int32 MinLevel = Chosen ? Chosen->MinItemLevel : 1; // stand-in: level 1 if LootLevelData lacks matching category
	int32 MaxLevel = Chosen ? Chosen->MaxItemLevel : 1;
	if (MinLevel < 1)
		MinLevel = 1;
	if (MaxLevel < MinLevel)
		MaxLevel = MinLevel;

	if (ItemMax > 0)
	{
		if (ItemMin > 0)
			MinLevel = FMath::Max(MinLevel, ItemMin);
		MaxLevel = FMath::Min(MaxLevel, ItemMax);
		if (MaxLevel < MinLevel)
			MaxLevel = MinLevel;
	}

	const int32 Span = MaxLevel - MinLevel + 1;
	const int32 ClampedOffset = FMath::Clamp(static_cast<int32>(Random01 * static_cast<double>(Span)), 0, Span - 1);
	OutLevel = MinLevel + ClampedOffset;
	return OutLevel >= 0;
}

int32 FortAthenaLootSearch::ResolveLoadedAmmo(const UFortItemDefinition* Definition)
{
	const UFortWeaponItemDefinition* WeaponDef = Cast<const UFortWeaponItemDefinition>(Definition);
	if (!WeaponDef)
		return 0;

	const FDataTableRowHandle& StatHandle = WeaponDef->GetWeaponStatHandle();
	if (!StatHandle.DataTable || StatHandle.RowName == NAME_None)
		return 0;

	if (const FFortBaseWeaponStats* Stats = StatHandle.GetRow<FFortBaseWeaponStats>(TEXT("FortAthenaLootSearch::ResolveLoadedAmmo")))
		return FMath::Max(0, Stats->ClipSize);

	return 0;
}

FName FortAthenaLootSearch::ResolveContainerLootTierGroup(const ABuildingContainer* Container)
{
	if (!Container)
		return NAME_None;

	if (Container->ChosenRandomUpgrade >= 0 && Container->ChosenRandomUpgrade < Container->PotentialRandomUpgrades.Num())
	{
		const FName UpgradeGroup = Container->PotentialRandomUpgrades[Container->ChosenRandomUpgrade].LootTierGroupIfApplied;
		if (UpgradeGroup != NAME_None)
			return UpgradeGroup;
	}
	return Container->SearchLootTierGroup;
}

bool FortAthenaLootSearch::RollAndSpawnContainerLoot(
	AFortGameModeAthena* GameMode,
	ABuildingContainer* Container,
	FName TierGroup,
	const FVector& WorldOrigin,
	float ScatterRadius,
	TArray<AFortPickupAthena*>* OutPickups)
{
	if (!IsEnabled() || !GameMode || !GameMode->HasAuthority() || !Container || TierGroup == NAME_None)
		return false;

	UWorld* World = Container->GetWorld();
	if (!World || World->GetNetMode() == NM_Client)
		return false;

	if (!FortAthenaLootData::IsReady(GameMode))
	{
		UE_LOG(LogTemp, Warning, TEXT("LootSearch: cannot roll container=%s group=%s (loot data not ready)"),
			*Container->GetName(), *TierGroup.ToString());
		return false;
	}

	FortAthenaLootRoll::FRequest Req;
	Req.Group = TierGroup;
	Req.WorldLevel = FortAthenaLootData::GetLootLevel(GameMode);
	Req.LootTier = Container->ReplicatedLootTier;
	Req.LootLevel = Req.WorldLevel;
	Req.bRequestBonus = false;

	const UDataTable* LevelTable = FortAthenaLootData::GetLootLevelTable(GameMode);
	TArray<FortAthenaLootRoll::FDrop> Drops;
	FString RollError;

	const bool bRolled = FortAthenaLootRoll::RollForMode(
		GameMode,
		Req,
		[]() { return SampleUnitRandom(); },
		[LevelTable](UFortItemDefinition* Def, int32 LootLvl, int32& OutLvl)
		{
			return ResolveItemLevel(LevelTable, Def, LootLvl, SampleUnitRandom(), OutLvl);
		},
		Drops,
		RollError);

	if (!bRolled)
	{
		UE_LOG(LogTemp, Warning, TEXT("LootSearch: roll failed container=%s group=%s worldLevel=%d tier=%d err=%s"),
			*Container->GetName(), *TierGroup.ToString(), Req.WorldLevel, Req.LootTier, *RollError);
		return false;
	}

	int32 SpawnedCount = 0;
	for (int32 Index = 0; Index < Drops.Num(); ++Index)
	{
		const FortAthenaLootRoll::FDrop& Drop = Drops[Index];
		if (!Drop.Definition || Drop.Count <= 0)
			continue;

		const int32 LoadedAmmo = ResolveLoadedAmmo(Drop.Definition);
		const FVector SpawnLoc = ComputeDropSpawnLocation(Container, WorldOrigin, Index, Drops.Num(), ScatterRadius);
		AFortPickupAthena* Pickup = FortAthenaLoot::SpawnResolvedPickup(
			World, SpawnLoc, Drop.Definition, Drop.Count, Drop.Level, LoadedAmmo);
		if (Pickup)
		{
			++SpawnedCount;
			if (OutPickups)
				OutPickups->Add(Pickup);
			UE_LOG(LogTemp, Display, TEXT("LootSearch:  spawned item=%s count=%d level=%d ammo=%d loc=(%.0f,%.0f,%.0f)"),
				*Drop.Definition->GetName(), Drop.Count, Drop.Level, LoadedAmmo, SpawnLoc.X, SpawnLoc.Y, SpawnLoc.Z);
		}
	}

	UE_LOG(LogTemp, Display, TEXT("LootSearch: rolled container=%s group=%s worldLevel=%d drops=%d spawned=%d"),
		*Container->GetName(), *TierGroup.ToString(), Req.WorldLevel, Drops.Num(), SpawnedCount);
	return true;
}

bool FortAthenaLootSearch::TrySearchContainer(AFortPlayerController* Controller, ABuildingContainer* Container)
{
	if (!IsEnabled())
		return false;

	if (!Controller || !Controller->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("LootSearch: rejected (missing authoritative controller)"));
		return false;
	}

	FString RejectReason;
	if (!IsInteractableContainer(Container, RejectReason))
	{
		UE_LOG(LogTemp, Display, TEXT("LootSearch: rejected container=%s player=%s reason=%s"),
			*GetNameSafe(Container), *Controller->GetName(), *RejectReason);
		return false;
	}

	AFortPawn* Pawn = Controller->MyFortPawn ? Cast<AFortPawn>(Controller->MyFortPawn) : Cast<AFortPawn>(Controller->GetPawn());
	if (!IsValid(Pawn) || Pawn->GetWorld() != Container->GetWorld())
	{
		UE_LOG(LogTemp, Warning, TEXT("LootSearch: rejected container=%s player=%s (missing or mismatched pawn)"),
			*Container->GetName(), *Controller->GetName());
		return false;
	}

	float Dist2D = 0.0f;
	float DistZ = 0.0f;
	if (!PassesDistanceCheck(Pawn, Container, Dist2D, DistZ))
	{
		UE_LOG(LogTemp, Display, TEXT("LootSearch: rejected container=%s player=%s out_of_range dist2D=%.1f distZ=%.1f"),
			*Container->GetName(), *Controller->GetName(), Dist2D, DistZ);
		return false;
	}

	UWorld* World = Container->GetWorld();
	AFortGameModeAthena* GameMode = World ? World->GetAuthGameMode<AFortGameModeAthena>() : nullptr;
	if (!GameMode || !FortAthenaLootData::IsReady(GameMode))
	{
		UE_LOG(LogTemp, Warning, TEXT("LootSearch: rejected container=%s player=%s (loot data not initialized)"),
			*Container->GetName(), *Controller->GetName());
		return false;
	}

	const FName TierGroup = ResolveContainerLootTierGroup(Container);
	if (TierGroup == NAME_None)
	{
		UE_LOG(LogTemp, Display, TEXT("LootSearch: rejected container=%s player=%s empty_tier_group upgrade=%d"),
			*Container->GetName(), *Controller->GetName(), Container->ChosenRandomUpgrade);
		return false;
	}

	// Transform container-local LootSpawnLocation_Athena (e.g. Chest=(0,50,30), Ammo=(0,35,10)) to world space.
	const FVector WorldSpawn = Container->GetActorTransform().TransformPosition(Container->LootSpawnLocation_Athena);
	const float ScatterRadius = Container->SearchBounceRadiusOverride > 0.0f
		? FMath::Min(Container->SearchBounceRadiusOverride, 80.0f)
		: 45.0f; // stand-in: container toss scatter when LootTossSpeed_Athena flight isn't active

	UE_LOG(LogTemp, Display, TEXT("LootSearch: rolling container=%s class=%s group=%s upgrade=%d tossSpeed=%.0f tossCone=%.0f"),
		*Container->GetName(), *GetNameSafe(Container->GetClass()), *TierGroup.ToString(),
		Container->ChosenRandomUpgrade, Container->LootTossSpeed_Athena, Container->LootTossConeHalfAngle_Athena);

	if (!RollAndSpawnContainerLoot(GameMode, Container, TierGroup, WorldSpawn, ScatterRadius))
		return false;

	// Only mark searched / destroy once roll succeeded so unready data does not silently eat chests.
	CommitContainerSearched(Container, Pawn);
	return true;
}
