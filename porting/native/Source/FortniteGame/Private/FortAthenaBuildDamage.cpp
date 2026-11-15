#include "FortAthenaBuildDamage.h"

#include "ABuildingActor.h"
#include "ABuildingSMActor.h"
#include "AFortInventory.h"
#include "AFortPawn.h"
#include "AFortPlayerController.h"
#include "AFortPlayerState.h"
#include "AFortPlayerStateAthena.h"
#include "AFortPlayerStateZone.h"
#include "Components/PrimitiveComponent.h"
#include "EFortResourceType.h"
#include "Engine/CurveTable.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "FortAthenaInventoryFlow.h"
#include "FortHarvestBuilding.h"
#include "FortHarvestDamage.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UFortAssetManager.h"
#include "UFortGameData.h"
#include "UFortResourceItemDefinition.h"
#include "UFortWorldItem.h"

DEFINE_LOG_CATEGORY_STATIC(LogFortAthenaBuildDamage, Log, All);

namespace FortAthenaBuildDamage
{
namespace
{
	const TCHAR* BuildDamageAttributesPropsPath = TEXT("/Game/Balance/DataTables/AttributesBuildingProps.AttributesBuildingProps");
	const TCHAR* BuildDamageAthenaSectionPath = TEXT("/Game/Athena/Balance/DataTables/AthenaAttributesBuildingSection.AthenaAttributesBuildingSection");
	const TCHAR* BuildDamageDefaultGameDataTablePath = TEXT("/Game/Balance/DataTables/DefaultGameData.DefaultGameData");
	const TCHAR* BuildDamageAthenaGameDataTablePath = TEXT("/Game/Athena/Balance/DataTables/AthenaGameData.AthenaGameData");

	struct FBuildDamageProgressEntry
	{
		TWeakObjectPtr<ABuildingSMActor> Piece;
		float RatePerSecond = 0.0f;
		bool bIsRepair = false;
	};

	TArray<FBuildDamageProgressEntry> BuildDamageActiveProgress;

	void BuildDamageLogDecision(const FString& Detail)
	{
		UE_LOG(LogFortAthenaBuildDamage, Display, TEXT("Build: %s"), *Detail);
	}

	UFortGameData* BuildDamageGetGameDataCommon()
	{
		UFortAssetManager* Manager = GEngine ? Cast<UFortAssetManager>(GEngine->AssetManager) : nullptr;
		return Manager ? Manager->GameDataCommon : nullptr;
	}

	int32 BuildDamageGetResourceIndex(EFortResourceType::Type ResourceType)
	{
		const int32 Index = static_cast<int32>(ResourceType);
		return (Index >= 0 && Index < 5) ? Index : -1;
	}

	const TCHAR* BuildDamageGetResourceCurveSuffix(EFortResourceType::Type ResourceType)
	{
		switch (ResourceType)
		{
		case EFortResourceType::Wood: return TEXT("Wood");
		case EFortResourceType::Stone: return TEXT("Stone");
		case EFortResourceType::Metal: return TEXT("Metal");
		case EFortResourceType::Permanite: return TEXT("Permanite");
		default: return nullptr;
		}
	}

	// Confidence: data + binary (0x141ba73d0 / AthenaAttributesBuildingSection.uasset).
	// Evaluates FortBuildingActorSet.BuildTime or RepairTime for a piece's AttributeInitKeys
	// (e.g. AthenaPlayerBuildingWood1.Size9.FortBuildingActorSet.BuildTime = 4, Stone = 12, Metal = 25).
	float BuildDamageEvalBuildingSetTime(const ABuildingSMActor* Building, const TCHAR* AttributeName, float FallbackSeconds)
	{
		if (!Building)
		{
			return FallbackSeconds;
		}

		UCurveTable* Tables[2] = {
			LoadObject<UCurveTable>(nullptr, BuildDamageAthenaSectionPath),
			LoadObject<UCurveTable>(nullptr, BuildDamageAttributesPropsPath),
		};

		const float Level = static_cast<float>(FMath::Max(0, Building->GetCurrentBuildingLevel()));
		for (int32 KeyIndex = 0; KeyIndex < 2; ++KeyIndex)
		{
			const FFortAttributeInitializationKey& Key = Building->AttributeInitKeys[KeyIndex];
			if (Key.AttributeInitCategory.IsNone() || Key.AttributeInitSubCategory.IsNone())
			{
				continue;
			}
			const FName RowName(*FString::Printf(
				TEXT("%s.%s.FortBuildingActorSet.%s"),
				*Key.AttributeInitCategory.ToString(),
				*Key.AttributeInitSubCategory.ToString(),
				AttributeName));

			for (UCurveTable* Table : Tables)
			{
				if (!Table)
				{
					continue;
				}
				if (const FRealCurve* Curve = Table->FindCurve(RowName, TEXT("BuildDamageTimeLookup"), false))
				{
					const float Value = Curve->Eval(Level);
					if (Value > 0.0f)
					{
						return Value;
					}
				}
			}
		}

		// stand-in: cooked material fallback when AttributeInitKeys does not carry an AthenaPlayerBuilding* row.
		switch (Building->ResourceType.GetValue())
		{
		case EFortResourceType::Wood: return 4.0f;
		case EFortResourceType::Stone: return 12.0f;
		case EFortResourceType::Metal: return 25.0f;
		default: return FallbackSeconds;
		}
	}

	// Confidence: data + binary (0x141e5d8e0 / DefaultGameData.uasset).
	// Reads BuildingCost_<Resource> (10 at level 0) via GameDataCommon->BuildingCostHandles or direct DefaultGameData table.
	float BuildDamageGetBuildingCost(EFortResourceType::Type ResourceType, int32 BuildingLevel)
	{
		const int32 Index = BuildDamageGetResourceIndex(ResourceType);
		const float Level = static_cast<float>(FMath::Max(0, BuildingLevel));

		if (Index >= 0)
		{
			if (UFortGameData* GameData = BuildDamageGetGameDataCommon())
			{
				const FCurveTableRowHandle& Handle = GameData->BuildingCostHandles[Index];
				if (Handle.CurveTable && !Handle.RowName.IsNone())
				{
					if (const FRealCurve* Curve = Handle.CurveTable->FindCurve(Handle.RowName, TEXT("BuildingCost"), false))
					{
						return FMath::Max(0.0f, Curve->Eval(Level));
					}
				}
			}
		}

		if (const TCHAR* Suffix = BuildDamageGetResourceCurveSuffix(ResourceType))
		{
			if (UCurveTable* Table = LoadObject<UCurveTable>(nullptr, BuildDamageDefaultGameDataTablePath))
			{
				const FName RowName(*FString::Printf(TEXT("BuildingCost_%s"), Suffix));
				if (const FRealCurve* Curve = Table->FindCurve(RowName, TEXT("BuildingCostFallback"), false))
				{
					return FMath::Max(0.0f, Curve->Eval(Level));
				}
			}
		}

		return 10.0f; // data fallback matching DefaultGameData BuildingCost_*
	}

	// Confidence: data + binary (0x141e5d8e0 / DefaultGameData.uasset).
	// Reads BuildingRepairCostMult_<Resource> (0.75 at level 0) via BuildingRepairCostMultiplierHandles.
	float BuildDamageGetRepairCostMultiplier(EFortResourceType::Type ResourceType, int32 BuildingLevel)
	{
		const int32 Index = BuildDamageGetResourceIndex(ResourceType);
		const float Level = static_cast<float>(FMath::Max(0, BuildingLevel));

		if (Index >= 0)
		{
			if (UFortGameData* GameData = BuildDamageGetGameDataCommon())
			{
				const FCurveTableRowHandle& Handle = GameData->BuildingRepairCostMultiplierHandles[Index];
				if (Handle.CurveTable && !Handle.RowName.IsNone())
				{
					if (const FRealCurve* Curve = Handle.CurveTable->FindCurve(Handle.RowName, TEXT("BuildingRepairCostMult"), false))
					{
						const float Value = Curve->Eval(Level);
						if (Value > 0.0f)
						{
							return Value;
						}
					}
				}
			}
		}

		if (const TCHAR* Suffix = BuildDamageGetResourceCurveSuffix(ResourceType))
		{
			if (UCurveTable* Table = LoadObject<UCurveTable>(nullptr, BuildDamageDefaultGameDataTablePath))
			{
				const FName RowName(*FString::Printf(TEXT("BuildingRepairCostMult_%s"), Suffix));
				if (const FRealCurve* Curve = Table->FindCurve(RowName, TEXT("BuildingRepairCostMultFallback"), false))
				{
					const float Value = Curve->Eval(Level);
					if (Value > 0.0f)
					{
						return Value;
					}
				}
			}
		}

		return 0.75f; // data fallback matching DefaultGameData BuildingRepairCostMult_*
	}

	// Confidence: data (AthenaGameData.uasset BuildingInitialHealthPercent_*).
	float BuildDamageGetInitialHealthPercent(const ABuildingSMActor* Building)
	{
		if (!Building)
		{
			return 1.0f;
		}

		const int32 Index = BuildDamageGetResourceIndex(Building->ResourceType.GetValue());
		const float Level = static_cast<float>(FMath::Max(0, Building->GetCurrentBuildingLevel()));
		if (Index >= 0)
		{
			if (UFortGameData* GameData = BuildDamageGetGameDataCommon())
			{
				const FCurveTableRowHandle& Handle = GameData->AthenaBuildingInitialHealthPercentHandles[Index];
				if (Handle.CurveTable && !Handle.RowName.IsNone())
				{
					if (const FRealCurve* Curve = Handle.CurveTable->FindCurve(Handle.RowName, TEXT("AthenaBuildingInitPct"), false))
					{
						const float Pct = Curve->Eval(Level);
						if (Pct > 0.0f)
						{
							return FMath::Clamp(Pct, 0.01f, 1.0f);
						}
					}
				}
			}
		}

		if (const TCHAR* Suffix = BuildDamageGetResourceCurveSuffix(Building->ResourceType.GetValue()))
		{
			if (UCurveTable* Table = LoadObject<UCurveTable>(nullptr, BuildDamageAthenaGameDataTablePath))
			{
				const FName RowName(*FString::Printf(TEXT("BuildingInitialHealthPercent_%s"), Suffix));
				if (const FRealCurve* Curve = Table->FindCurve(RowName, TEXT("AthenaBuildingInitPctFallback"), false))
				{
					const float Pct = Curve->Eval(Level);
					if (Pct > 0.0f)
					{
						return FMath::Clamp(Pct, 0.01f, 1.0f);
					}
				}
			}
		}

		switch (Building->ResourceType.GetValue())
		{
		case EFortResourceType::Wood: return 0.6f;
		case EFortResourceType::Stone: return 0.333333f;
		case EFortResourceType::Metal: return 0.22f;
		default: return 1.0f;
		}
	}

	void BuildDamageUnregisterProgress(const ABuildingSMActor* Building)
	{
		for (int32 i = BuildDamageActiveProgress.Num() - 1; i >= 0; --i)
		{
			ABuildingSMActor* EntryPiece = BuildDamageActiveProgress[i].Piece.Get();
			if (!EntryPiece || EntryPiece == Building)
			{
				BuildDamageActiveProgress.RemoveAtSwap(i, 1, false);
			}
		}
	}

	void BuildDamageRegisterProgress(ABuildingSMActor* Building, float RatePerSecond, bool bIsRepair)
	{
		if (!Building || RatePerSecond <= 0.0f)
		{
			return;
		}
		BuildDamageUnregisterProgress(Building);
		FBuildDamageProgressEntry Entry;
		Entry.Piece = Building;
		Entry.RatePerSecond = RatePerSecond;
		Entry.bIsRepair = bIsRepair;
		BuildDamageActiveProgress.Add(Entry);
	}

	int32 BuildDamageCountResourceInInventory(const AFortPlayerController* Controller, EFortResourceType::Type ResourceType)
	{
		const AFortInventory* Inventory = Controller ? Controller->WorldInventory : nullptr;
		if (!Inventory)
		{
			return 0;
		}

		int32 Total = 0;
		for (const UFortWorldItem* Item : Inventory->Inventory.ItemInstances)
		{
			const UFortResourceItemDefinition* ResDef = Item
				? Cast<UFortResourceItemDefinition>(Item->ItemEntry.ItemDefinition)
				: nullptr;
			if (ResDef && ResDef->ResourceType == ResourceType && Item->ItemEntry.Count > 0)
			{
				Total += Item->ItemEntry.Count;
			}
		}
		return Total;
	}

	bool BuildDamageConsumeResourceFromInventory(AFortPlayerController* Controller, EFortResourceType::Type ResourceType, int32 AmountToConsume)
	{
		if (AmountToConsume <= 0)
		{
			return true;
		}

		if (BuildDamageCountResourceInInventory(Controller, ResourceType) < AmountToConsume)
		{
			return false;
		}

		AFortInventory* Inventory = Controller ? Controller->WorldInventory : nullptr;
		if (!Inventory)
		{
			return false;
		}

		TArray<TPair<FGuid, int32>> Deductions;
		int32 Remaining = AmountToConsume;
		for (const UFortWorldItem* Item : Inventory->Inventory.ItemInstances)
		{
			if (Remaining <= 0)
			{
				break;
			}
			const UFortResourceItemDefinition* ResDef = Item
				? Cast<UFortResourceItemDefinition>(Item->ItemEntry.ItemDefinition)
				: nullptr;
			if (ResDef && ResDef->ResourceType == ResourceType && Item->ItemEntry.Count > 0)
			{
				const int32 Take = FMath::Min(Remaining, Item->ItemEntry.Count);
				Deductions.Emplace(Item->ItemEntry.ItemGuid, Take);
				Remaining -= Take;
			}
		}

		for (const TPair<FGuid, int32>& Step : Deductions)
		{
			FortAthenaInventory::RemoveItem(Controller, Step.Key, Step.Value);
		}
		return Remaining == 0;
	}

	float BuildDamageGetMaxInteractionDistance(const AFortPlayerController* Controller)
	{
		if (Controller && Controller->EditModeDistance > 0.0f)
		{
			return Controller->EditModeDistance;
		}
		if (const UFortGameData* GameData = BuildDamageGetGameDataCommon())
		{
			if (GameData->EditModeCancelDistance > 0.0f)
			{
				return GameData->EditModeCancelDistance;
			}
		}
		return 1500.0f; // stand-in: ~3 grid cells when EditModeDistance/EditModeCancelDistance is unset
	}

	void BuildDamageDestroyPlayerPiece(
		ABuildingSMActor* Building,
		float Damage,
		AController* Instigator,
		AActor* Causer,
		const FHitResult& Hit)
	{
		BuildDamageUnregisterProgress(Building);

		if (Building->bUnderRepair)
		{
			Building->bUnderRepair = 0;
			Building->OnRep_bUnderRepair();
		}
		if (Building->EditingPlayer || Building->bCurrentlyBeingEdited)
		{
			Building->EditingPlayer = nullptr;
			Building->bCurrentlyBeingEdited = 0;
			Building->OnRep_EditingPlayer();
		}

		const FVector HitPoint = !Hit.ImpactPoint.IsZero() ? FVector(Hit.ImpactPoint) : Building->GetActorLocation();
		const FVector Momentum = FVector::ZeroVector;
		FGameplayTagContainer DamageTags;
		FGameplayEffectContextHandle EffectContext;
		FHitResult MutableHit = Hit;

		// Confidence: binary (0x141c0c330 -> 0x141c00e60 -> 0x141babc20), aligned with FortHarvestBuilding.cpp lines 633-648.
		Building->bDestroyed = 1;
		Building->OnDeathServer(Damage, DamageTags, Momentum, MutableHit, Instigator, Causer, EffectContext);
		Building->OnDied.Broadcast(Building, Damage, Instigator, Causer, HitPoint, Hit.Component.Get(), Hit.BoneName, Momentum);

		Building->SetActorEnableCollision(false);
		if (Building->bHideOnDeath)
		{
			Building->SetActorHiddenInGame(true);
		}
		Building->HandleDestroyed();

		// stand-in: LifespanAfterDeath comes from the native constructor default, which is not recovered. A guessed 5 s left
	// destroyed objects standing (hits kept landing on them with no weak spots, then they popped out); retail removes them
	// right after the death animation. -FortniteDeathLifespan=<seconds> overrides the fallback.
	static const float FallbackLifespan = []() { float V = 1.0f; FParse::Value(FCommandLine::Get(), TEXT("FortniteDeathLifespan="), V); return FMath::Max(0.05f, V); }();
	const float LifeSpan = (Building->LifespanAfterDeath > 0.0f) ? Building->LifespanAfterDeath : FallbackLifespan;
		Building->SetLifeSpan(LifeSpan);
		Building->ForceNetUpdate();
	}
} // namespace

bool IsEnabled()
{
	return FParse::Param(FCommandLine::Get(), TEXT("FortniteOriginalClient"));
}

// Confidence: binary + data (0x141baf090, 0x141c0d460, 0x1423881c0, AthenaAttributesBuildingSection, AthenaGameData).
void InitializePlayerBuiltPiece(ABuildingSMActor* Building, AFortPlayerController* OwnerPC)
{
	if (!IsEnabled() || !Building || !Building->IsValidLowLevel())
	{
		return;
	}

	Building->bPlayerPlaced = 1;
	// Always disallow resource drops on player-placed builds (FortHarvestFlow enables bAllowResourceDrop on ABuildingSMActor CDO for world walls).
	Building->bAllowResourceDrop = 0;

	if (OwnerPC && OwnerPC->PlayerState)
	{
		if (const AFortPlayerState* FortPS = Cast<AFortPlayerState>(OwnerPC->PlayerState))
			Building->OwnerPersistentID = FortPS->WorldPlayerId;
		if (AFortPlayerStateAthena* PS = Cast<AFortPlayerStateAthena>(OwnerPC->PlayerState))
		{
			Building->SetTeam(PS->TeamIndex);
		}
	}

	FortHarvest::InitializeBuildingHealth(Building);
	const float MaxHealth = FortHarvest::GetBuildingMaxHealth(Building);
	const float CurHealth = FortHarvest::GetBuildingHealth(Building);
	const float BuildTime = FMath::Max(0.1f, BuildDamageEvalBuildingSetTime(Building, TEXT("BuildTime"), 4.0f));

	const float AutoBuildRemaining = FMath::Max(0.0f, MaxHealth - CurHealth);
	Building->HealthToAutoBuild = AutoBuildRemaining;
	Building->bUnderConstruction = (AutoBuildRemaining > KINDA_SMALL_NUMBER) ? 1 : 0;

	if (Building->bUnderConstruction)
	{
		const float InitPct = BuildDamageGetInitialHealthPercent(Building);
		const float TotalGain = FMath::Max(1.0f, MaxHealth * FMath::Max(0.05f, 1.0f - InitPct));
		BuildDamageRegisterProgress(Building, TotalGain / BuildTime, false);
	}

	Building->ForceNetUpdate();

	BuildDamageLogDecision(FString::Printf(
		TEXT("InitializePlayerBuiltPiece %s res=%d ownerId=%d team=%u health=%.1f/%.1f autoBuild=%.1f buildTime=%.1fs"),
		*Building->GetName(),
		static_cast<int32>(Building->ResourceType.GetValue()),
		static_cast<int32>(Building->OwnerPersistentID),
		static_cast<uint32>(Building->TeamIndex),
		CurHealth,
		MaxHealth,
		AutoBuildRemaining,
		BuildTime));
}

// Confidence: binary (0x141c0d460 lines 225-245 / 0x141c19240).
void InitializeEditedBuildingPiece(ABuildingSMActor* OldBuilding, ABuildingSMActor* NewBuilding, AFortPlayerController* OwnerPC)
{
	if (!IsEnabled() || !NewBuilding || !NewBuilding->IsValidLowLevel())
	{
		return;
	}

	if (!OldBuilding || !OldBuilding->IsValidLowLevel())
	{
		InitializePlayerBuiltPiece(NewBuilding, OwnerPC);
		return;
	}

	BuildDamageUnregisterProgress(OldBuilding);

	NewBuilding->bPlayerPlaced = OldBuilding->bPlayerPlaced;
	if (NewBuilding->bPlayerPlaced)
	{
		NewBuilding->bAllowResourceDrop = 0;
	}
	NewBuilding->OwnerPersistentID = OldBuilding->OwnerPersistentID;

	const float OldMax = FMath::Max(1.0f, FortHarvest::GetBuildingMaxHealth(OldBuilding));
	const float OldCur = FMath::Clamp(FortHarvest::GetBuildingHealth(OldBuilding), 1.0f, OldMax);
	const float OldPct = FMath::Clamp(OldCur / OldMax, 0.01f, 1.0f);
	const float OldTargetPct = FMath::Clamp((OldCur + FMath::Max(0.0f, OldBuilding->HealthToAutoBuild)) / OldMax, OldPct, 1.0f);

	FortHarvest::InitializeBuildingHealth(NewBuilding);
	const float NewMax = FMath::Max(1.0f, FortHarvest::GetBuildingMaxHealth(NewBuilding));
	const float NewHealth = FMath::Clamp(NewMax * OldPct, 1.0f, NewMax);
	const float NewTargetHealth = FMath::Clamp(NewMax * OldTargetPct, NewHealth, NewMax);

	FortHarvest::ForceBuildingHealth(NewBuilding, NewHealth);
	NewBuilding->HealthToAutoBuild = FMath::Max(0.0f, NewTargetHealth - NewHealth);
	NewBuilding->bUnderConstruction = (NewBuilding->HealthToAutoBuild > KINDA_SMALL_NUMBER) ? 1 : 0;

	if (NewBuilding->bUnderConstruction)
	{
		const float BuildTime = FMath::Max(0.1f, BuildDamageEvalBuildingSetTime(NewBuilding, TEXT("BuildTime"), 4.0f));
		const float InitPct = BuildDamageGetInitialHealthPercent(NewBuilding);
		const float TotalGain = FMath::Max(1.0f, NewMax * FMath::Max(0.05f, 1.0f - InitPct));
		BuildDamageRegisterProgress(NewBuilding, TotalGain / BuildTime, false);
	}

	NewBuilding->ForceNetUpdate();

	BuildDamageLogDecision(FString::Printf(
		TEXT("InitializeEditedBuildingPiece %s -> %s oldPct=%.3f newHealth=%.1f/%.1f autoBuild=%.1f"),
		*OldBuilding->GetName(),
		*NewBuilding->GetName(),
		OldPct,
		NewHealth,
		NewMax,
		NewBuilding->HealthToAutoBuild));
}

// Confidence: data + inferred (uses BuildTime/RepairTime curves from AthenaAttributesBuildingSection).
void TickAutoBuild(UWorld* World, float DeltaSeconds)
{
	if (!IsEnabled() || !World || DeltaSeconds <= 0.0f || BuildDamageActiveProgress.Num() == 0)
	{
		return;
	}

	for (int32 i = BuildDamageActiveProgress.Num() - 1; i >= 0; --i)
	{
		FBuildDamageProgressEntry& Entry = BuildDamageActiveProgress[i];
		ABuildingSMActor* Piece = Entry.Piece.Get();
		if (!Piece || !Piece->IsValidLowLevel() || Piece->bDestroyed || Piece->GetWorld() != World)
		{
			BuildDamageActiveProgress.RemoveAtSwap(i, 1, false);
			continue;
		}

		const float MaxHealth = FortHarvest::GetBuildingMaxHealth(Piece);
		const float CurHealth = FortHarvest::GetBuildingHealth(Piece);
		if (MaxHealth <= 0.0f || CurHealth >= MaxHealth - 0.1f)
		{
			if (MaxHealth > 0.0f && CurHealth < MaxHealth)
			{
				FortHarvest::ForceBuildingHealth(Piece, MaxHealth);
			}
			const bool bWasRepair = Entry.bIsRepair || (Piece->bUnderRepair != 0);
			Piece->HealthToAutoBuild = 0.0f;
			if (Piece->bUnderConstruction)
			{
				Piece->bUnderConstruction = 0;
				Piece->OnRep_bUnderConstruction();
			}
			if (Piece->bUnderRepair)
			{
				Piece->bUnderRepair = 0;
				Piece->OnRep_bUnderRepair();
				Piece->OnRepairBuildingFinished.Broadcast();
			}
			Piece->ForceNetUpdate();
			BuildDamageLogDecision(FString::Printf(
				TEXT("%s finished %s health=%.1f/%.1f"),
				bWasRepair ? TEXT("Repair") : TEXT("AutoBuild"),
				*Piece->GetName(),
				FortHarvest::GetBuildingHealth(Piece),
				MaxHealth));
			BuildDamageActiveProgress.RemoveAtSwap(i, 1, false);
			continue;
		}

		const float Step = FMath::Max(0.0f, Entry.RatePerSecond * DeltaSeconds);
		if (Step <= 0.0f)
		{
			continue;
		}

		if (Entry.bIsRepair)
		{
			if (!Piece->bUnderRepair)
			{
				BuildDamageActiveProgress.RemoveAtSwap(i, 1, false);
				continue;
			}
			const float NextHealth = FMath::Min(MaxHealth, CurHealth + Step);
			FortHarvest::ForceBuildingHealth(Piece, NextHealth);
			if (NextHealth >= MaxHealth - 0.1f)
			{
				FortHarvest::ForceBuildingHealth(Piece, MaxHealth);
				Piece->bUnderRepair = 0;
				Piece->OnRep_bUnderRepair();
				Piece->OnRepairBuildingFinished.Broadcast();
				Piece->ForceNetUpdate();
				BuildDamageLogDecision(FString::Printf(TEXT("Repair finished %s health=%.1f/%.1f"), *Piece->GetName(), MaxHealth, MaxHealth));
				BuildDamageActiveProgress.RemoveAtSwap(i, 1, false);
			}
		}
		else
		{
			if (!Piece->bUnderConstruction || Piece->HealthToAutoBuild <= 0.0f)
			{
				Piece->HealthToAutoBuild = 0.0f;
				Piece->bUnderConstruction = 0;
				Piece->OnRep_bUnderConstruction();
				Piece->ForceNetUpdate();
				BuildDamageActiveProgress.RemoveAtSwap(i, 1, false);
				continue;
			}

			const float Applied = FMath::Min3(Step, Piece->HealthToAutoBuild, FMath::Max(0.0f, MaxHealth - CurHealth));
			const float NextHealth = FMath::Min(MaxHealth, CurHealth + Applied);
			Piece->HealthToAutoBuild = FMath::Max(0.0f, Piece->HealthToAutoBuild - Applied);
			FortHarvest::ForceBuildingHealth(Piece, NextHealth);

			if (Piece->HealthToAutoBuild <= 0.05f || NextHealth >= MaxHealth - 0.1f)
			{
				Piece->HealthToAutoBuild = 0.0f;
				Piece->bUnderConstruction = 0;
				Piece->OnRep_bUnderConstruction();
				Piece->ForceNetUpdate();
				BuildDamageLogDecision(FString::Printf(TEXT("AutoBuild finished %s health=%.1f/%.1f"), *Piece->GetName(), NextHealth, MaxHealth));
				BuildDamageActiveProgress.RemoveAtSwap(i, 1, false);
			}
		}
	}
}

bool ShouldAllowResourceDrop(const ABuildingSMActor* Building)
{
	return Building && !Building->bPlayerPlaced;
}

// Confidence: source (delegates to FortHarvest::ComputeMeleeEnvironmentDamage, FortHarvestDamage.{h,cpp}).
bool ComputePickaxeDamageForPiece(
	const ABuildingSMActor* Building,
	FName WeaponRow,
	uint8 AttackerTeam,
	bool bWeakSpot,
	float& OutDamage)
{
	OutDamage = 0.0f;
	if (!Building)
	{
		return false;
	}

	const FortHarvest::ETeamRelation Relation = (Building->TeamIndex == AttackerTeam)
		? FortHarvest::ETeamRelation::Friendly
		: FortHarvest::ETeamRelation::Hostile;

	FortHarvest::EHitSurface Surface = FortHarvest::EHitSurface::Default;
	if (bWeakSpot)
	{
		Surface = FortHarvest::EHitSurface::WeakSpot;
	}
	else
	{
		switch (Building->ResourceType.GetValue())
		{
		case EFortResourceType::Wood: Surface = FortHarvest::EHitSurface::Wood; break;
		case EFortResourceType::Stone: Surface = FortHarvest::EHitSurface::Stone; break;
		case EFortResourceType::Metal: Surface = FortHarvest::EHitSurface::Metal; break;
		default: Surface = FortHarvest::EHitSurface::Default; break;
		}
	}

	return FortHarvest::ComputeMeleeEnvironmentDamage(WeaponRow, Surface, Relation, OutDamage);
}

// Confidence: binary + source (0x141c0c330, 0x141baaf60, aligned with FortHarvestBuilding::ApplyBuildingDamage).
bool OnPlayerBuiltPieceDamaged(
	ABuildingSMActor* Building,
	float Damage,
	AController* Instigator,
	AActor* Causer,
	const FHitResult& Hit,
	bool bWeakSpot)
{
	if (!IsEnabled() || !Building || !Building->IsValidLowLevel() || Building->bDestroyed || Damage <= 0.0f)
	{
		return false;
	}

	// Ensure bAllowResourceDrop never leaks onto player-built pieces.
	if (Building->bPlayerPlaced)
	{
		Building->bAllowResourceDrop = 0;
	}

	if (FortHarvest::GetBuildingMaxHealth(Building) <= 0.0f)
	{
		FortHarvest::InitializeBuildingHealth(Building);
	}

	const float CurrentHealth = FortHarvest::GetBuildingHealth(Building);
	const float ClampedDamage = FMath::Min(Damage, FMath::Max(0.0f, CurrentHealth));
	const float NewHealth = FMath::Max(0.0f, CurrentHealth - ClampedDamage);
	const bool bWillDestroy = (NewHealth <= 0.0f);

	if (Building->bUnderRepair)
	{
		Building->bUnderRepair = 0;
		Building->OnRep_bUnderRepair();
		BuildDamageUnregisterProgress(Building);
	}

	FortHarvest::ForceBuildingHealth(Building, NewHealth);
	FortHarvest::ReplicateBuildingDamageCue(Building, ClampedDamage, Instigator, Causer, Hit, bWillDestroy);

	const FVector HitPoint = !Hit.ImpactPoint.IsZero() ? FVector(Hit.ImpactPoint) : Building->GetActorLocation();
	const FVector Momentum = FVector::ZeroVector;
	FGameplayTagContainer DamageTags;
	FGameplayEffectContextHandle EffectContext;
	FHitResult MutableHit = Hit;

	Building->OnDamageServer(ClampedDamage, DamageTags, Momentum, MutableHit, Instigator, Causer, EffectContext);
	Building->OnDamaged.Broadcast(Building, ClampedDamage, Instigator, Causer, HitPoint, Hit.Component.Get(), Hit.BoneName, Momentum);

	if (AFortPlayerController* PC = Cast<AFortPlayerController>(Instigator))
	{
		// Suppress resource reward for player-built pieces: EFortResourceType::None, count=0.
		PC->ClientReportDamagedResourceBuilding(Building, EFortResourceType::None, 0, bWillDestroy, bWeakSpot);
	}

	BuildDamageLogDecision(FString::Printf(
		TEXT("PlayerPieceDamaged %s dmg=%.1f health=%.1f->%.1f weakspot=%d destroyed=%d instigator=%s"),
		*Building->GetName(),
		ClampedDamage,
		CurrentHealth,
		NewHealth,
		bWeakSpot ? 1 : 0,
		bWillDestroy ? 1 : 0,
		Instigator ? *Instigator->GetName() : TEXT("none")));

	if (bWillDestroy)
	{
		BuildDamageDestroyPlayerPiece(Building, ClampedDamage, Instigator, Causer, Hit);
	}
	else
	{
		Building->ForceNetUpdate();
	}

	return true;
}

// Confidence: binary + data (0x141c18db0 + 0x141e5d8e0 + DefaultGameData).
int32 ComputeRepairCost(const ABuildingSMActor* Building)
{
	if (!Building || !Building->IsValidLowLevel() || Building->bDestroyed || !Building->bPlayerPlaced)
	{
		return 0;
	}

	const EFortResourceType::Type ResType = Building->ResourceType.GetValue();
	if (ResType >= EFortResourceType::None)
	{
		return 0;
	}

	const float MaxHealth = FortHarvest::GetBuildingMaxHealth(const_cast<ABuildingSMActor*>(Building));
	const float CurHealth = FortHarvest::GetBuildingHealth(Building);
	if (MaxHealth <= 0.0f || CurHealth >= MaxHealth - 0.1f)
	{
		return 0;
	}

	const float MissingFraction = FMath::Clamp((MaxHealth - CurHealth) / MaxHealth, 0.0f, 1.0f);
	const int32 Level = FMath::Max(0, Building->GetCurrentBuildingLevel());
	const float BuildCost = BuildDamageGetBuildingCost(ResType, Level);
	const float RepairMult = BuildDamageGetRepairCostMultiplier(ResType, Level);
	return FMath::Max(1, FMath::CeilToInt(BuildCost * MissingFraction * RepairMult));
}

// Confidence: binary + inferred (0x142a4c840, 0x141c18db0, 0x141c1b910, 0x141c104d0).
bool RepairBuildingActor(AFortPlayerController* Controller, ABuildingSMActor* Building)
{
	if (!IsEnabled())
	{
		return false;
	}
	if (!Controller || !Building || !Building->IsValidLowLevel())
	{
		BuildDamageLogDecision(TEXT("RepairBuildingActor refused: null controller or building"));
		return false;
	}
	if (Building->bDestroyed)
	{
		BuildDamageLogDecision(FString::Printf(TEXT("RepairBuildingActor refused: %s destroyed"), *Building->GetName()));
		return false;
	}
	if (!Building->bPlayerPlaced && !Building->bSupportsRepairing)
	{
		BuildDamageLogDecision(FString::Printf(TEXT("RepairBuildingActor refused: %s not playerPlaced/repairable"), *Building->GetName()));
		return false;
	}
	if (Building->bCurrentlyBeingEdited || Building->EditingPlayer != nullptr)
	{
		BuildDamageLogDecision(FString::Printf(TEXT("RepairBuildingActor refused: %s currently being edited"), *Building->GetName()));
		return false;
	}
	if (Building->bUnderRepair)
	{
		BuildDamageLogDecision(FString::Printf(TEXT("RepairBuildingActor refused: %s already under repair"), *Building->GetName()));
		return false;
	}

	const AFortPlayerStateAthena* PS = Cast<AFortPlayerStateAthena>(Controller->PlayerState);
	if (!PS || (PS->TeamIndex != Building->TeamIndex && (PS->WorldPlayerId == 0 || PS->WorldPlayerId != Building->OwnerPersistentID)))
	{
		BuildDamageLogDecision(FString::Printf(
			TEXT("RepairBuildingActor refused: %s permission failed pcTeam=%u bldgTeam=%u"),
			*Building->GetName(),
			PS ? static_cast<uint32>(PS->TeamIndex) : 255U,
			static_cast<uint32>(Building->TeamIndex)));
		return false;
	}

	if (const APawn* Pawn = Controller->GetPawn())
	{
		const float MaxDist = BuildDamageGetMaxInteractionDistance(Controller) * 1.15f;
		const float DistSq = FVector::DistSquared(Pawn->GetActorLocation(), Building->GetActorLocation());
		if (DistSq > MaxDist * MaxDist)
		{
			BuildDamageLogDecision(FString::Printf(
				TEXT("RepairBuildingActor refused: %s out of range dist=%.1f max=%.1f"),
				*Building->GetName(),
				FMath::Sqrt(DistSq),
				MaxDist));
			return false;
		}
	}

	if (FortHarvest::GetBuildingMaxHealth(Building) <= 0.0f)
	{
		FortHarvest::InitializeBuildingHealth(Building);
	}

	const float MaxHealth = FortHarvest::GetBuildingMaxHealth(Building);
	const float CurHealth = FortHarvest::GetBuildingHealth(Building);
	if (MaxHealth <= 0.0f || CurHealth >= MaxHealth - 0.1f)
	{
		BuildDamageLogDecision(FString::Printf(
			TEXT("RepairBuildingActor refused: %s full health %.1f/%.1f"),
			*Building->GetName(), CurHealth, MaxHealth));
		return false;
	}

	const EFortResourceType::Type ResType = Building->ResourceType.GetValue();
	const int32 RepairCost = Controller->bCraftFree ? 0 : ComputeRepairCost(Building);
	if (RepairCost > 0)
	{
		const int32 Available = BuildDamageCountResourceInInventory(Controller, ResType);
		if (Available < RepairCost || !BuildDamageConsumeResourceFromInventory(Controller, ResType, RepairCost))
		{
			BuildDamageLogDecision(FString::Printf(
				TEXT("RepairBuildingActor refused: %s insufficient res=%d avail=%d need=%d"),
				*Building->GetName(),
				static_cast<int32>(ResType),
				Available,
				RepairCost));
			return false;
		}
	}

	const float RepairTime = BuildDamageEvalBuildingSetTime(Building, TEXT("RepairTime"), 4.0f);
	Building->bUnderRepair = 1;
	Building->OnRep_bUnderRepair();
	Building->OnRepairStarted();
	Building->OnRepairBuildingStarted.Broadcast(Controller);

	if (RepairTime > 0.05f)
	{
		BuildDamageRegisterProgress(Building, MaxHealth / RepairTime, true);
	}
	else
	{
		FortHarvest::ForceBuildingHealth(Building, MaxHealth);
		Building->bUnderRepair = 0;
		Building->OnRep_bUnderRepair();
		Building->OnRepairBuildingFinished.Broadcast();
	}

	Building->ForceNetUpdate();

	BuildDamageLogDecision(FString::Printf(
		TEXT("RepairBuildingActor started %s by %s cost=%d res=%d health=%.1f/%.1f repairTime=%.1fs"),
		*Building->GetName(),
		*Controller->GetName(),
		RepairCost,
		static_cast<int32>(ResType),
		CurHealth,
		MaxHealth,
		RepairTime));
	return true;
}
} // namespace FortAthenaBuildDamage
