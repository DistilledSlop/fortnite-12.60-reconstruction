#include "FortHarvestBuilding.h"
#include "FortAthenaBuildDamage.h"

#include "ABuildingActor.h"
#include "ABuildingSMActor.h"
#include "AFortPlayerController.h"
#include "AbilitySystemGlobals.h"
#include "EAbilitySystemComponentCreationPolicy.h"
#include "EFortResourceType.h"
#include "Engine/CurveTable.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "FBuildingWeakSpotData.h"
#include "FFortAttributeInitializationKey.h"
#include "FortAthenaInventoryFlow.h"
#include "FortAthenaLoot.h"
#include "FortAttributeSetInitter.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UFortAbilitySystemComponent.h"
#include "UFortAssetManager.h"
#include "UFortBuildingActorSet.h"
#include "UFortGameData.h"
#include "UFortHealthSet.h"
#include "UFortItemDefinition.h"
#include "UFortResourceItemDefinition.h"
#include "UObject/UnrealType.h"

DEFINE_LOG_CATEGORY_STATIC(LogFortHarvestBuilding, Log, All);

namespace FortHarvest
{
namespace
{
	const TCHAR* AttributesBuildingPropsPath = TEXT("/Game/Balance/DataTables/AttributesBuildingProps.AttributesBuildingProps");
	const TCHAR* AthenaAttributesBuildingSectionPath = TEXT("/Game/Athena/Balance/DataTables/AthenaAttributesBuildingSection.AthenaAttributesBuildingSection");
	const TCHAR* AthenaResourceRatesPath = TEXT("/Game/Athena/Balance/DataTables/AthenaResourceRates.AthenaResourceRates");

	const TCHAR* WoodItemPath = TEXT("/Game/Items/ResourcePickups/WoodItemData.WoodItemData");
	const TCHAR* StoneItemPath = TEXT("/Game/Items/ResourcePickups/StoneItemData.StoneItemData");
	const TCHAR* MetalItemPath = TEXT("/Game/Items/ResourcePickups/MetalItemData.MetalItemData");

	FGameplayAttribute GetHealthAttribute()
	{
		static FProperty* Prop = UFortHealthSet::StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UFortHealthSet, Health));
		return FGameplayAttribute(Prop);
	}

	FGameplayAttribute GetMaxHealthAttribute()
	{
		static FProperty* Prop = UFortHealthSet::StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UFortHealthSet, MaxHealth));
		return FGameplayAttribute(Prop);
	}

	FFortAttributeSetInitter& GetBuildingAttributeInitter()
	{
		static FFortAttributeSetInitter BuildingInitter;
		static bool bInitialized = false;
		if (!bInitialized)
		{
			bInitialized = true;
			TArray<UCurveTable*> Tables;
			if (UCurveTable* PropsTable = LoadObject<UCurveTable>(nullptr, AttributesBuildingPropsPath))
			{
				Tables.Add(PropsTable);
			}
			if (UCurveTable* AthenaSectionTable = LoadObject<UCurveTable>(nullptr, AthenaAttributesBuildingSectionPath))
			{
				Tables.Add(AthenaSectionTable);
			}
			if (Tables.Num() > 0)
			{
				BuildingInitter.PreloadAttributeSetData(Tables);
			}
			else
			{
				UE_LOG(LogFortHarvestBuilding, Warning, TEXT("Building: Failed to load building attribute curve tables"));
			}
		}
		return BuildingInitter;
	}

	float ResolveMaxHealthFromCurves(const ABuildingActor* Building)
	{
		if (!Building)
		{
			return 0.0f;
		}

		// Original 0x141ba73d0: checks AttributeInitKeys against FFortAttributeSetInitter for UFortHealthSet::MaxHealth at GetAttributeInitLevel().
		const int32 Level = FMath::Max(0, Building->GetCurrentBuildingLevel());
		const FGameplayAttribute MaxHealthAttr = GetMaxHealthAttribute();
		const FAttributeSetInitter* GlobalInitter = UAbilitySystemGlobals::Get().GetAttributeSetInitter();

		for (int32 KeyIndex = 0; KeyIndex < 2; ++KeyIndex)
		{
			const FFortAttributeInitializationKey& Key = Building->AttributeInitKeys[KeyIndex];
			if (Key.AttributeInitCategory.IsNone() && Key.AttributeInitSubCategory.IsNone())
			{
				continue;
			}

			float OutValue = 0.0f;
			if (GlobalInitter)
			{
				const FFortAttributeSetInitter* FortGlobal = static_cast<const FFortAttributeSetInitter*>(GlobalInitter);
				if (FortGlobal->GetAttributeDefault(Key, MaxHealthAttr, Level, OutValue) && OutValue > 0.0f)
				{
					return OutValue;
				}
			}

			// stand-in: DefaultGame.ini does not configure GlobalAttributeSetDefaultsTableNames for building tables,
			// so query a preloaded FFortAttributeSetInitter holding AttributesBuildingProps + AthenaAttributesBuildingSection.
			if (GetBuildingAttributeInitter().GetAttributeDefault(Key, MaxHealthAttr, Level, OutValue) && OutValue > 0.0f)
			{
				return OutValue;
			}
		}

		return 0.0f;
	}

	float ResolveInitialHealthPct(const ABuildingActor* Building)
	{
		// Original 0x141c08670 / 0x141e5d550: non-player-placed props spawn at 1.0f; player-placed use GameData initial health handles.
		if (!Building || !Building->bPlayerPlaced)
		{
			return 1.0f;
		}

		const ABuildingSMActor* SMBuilding = Cast<ABuildingSMActor>(Building);
		if (!SMBuilding)
		{
			return 1.0f;
		}

		const int32 ResourceIndex = static_cast<int32>(SMBuilding->ResourceType.GetValue());
		if (ResourceIndex < 0 || ResourceIndex >= 5)
		{
			return 1.0f;
		}

		UFortAssetManager* Manager = GEngine ? Cast<UFortAssetManager>(GEngine->AssetManager) : nullptr;
		UFortGameData* GameData = Manager ? Manager->GameDataCommon : nullptr;
		if (!GameData)
		{
			return 1.0f;
		}

		const FCurveTableRowHandle& Handle = GameData->AthenaBuildingInitialHealthPercentHandles[ResourceIndex].CurveTable
			? GameData->AthenaBuildingInitialHealthPercentHandles[ResourceIndex]
			: GameData->BuildingInitialHealthPercentHandles[ResourceIndex];

		if (Handle.CurveTable && !Handle.RowName.IsNone())
		{
			if (const FRealCurve* Curve = Handle.CurveTable->FindCurve(Handle.RowName, TEXT("BuildingInitialHealthPct"), false))
			{
				const float Pct = Curve->Eval(static_cast<float>(Building->GetCurrentBuildingLevel()));
				if (Pct > 0.0f)
				{
					return FMath::Clamp(Pct, 0.01f, 1.0f);
				}
			}
		}

		return 1.0f;
	}

	// Level-placed harvestable prop: has a resource type and a resource-rate curve row and was not built by a player.
	// The cooked data never carries bAllowResourceDrop / bAllowWeakSpots / MaxResourcesToSpawn for these (they match the native
	// defaults, which the generated module does not reproduce for bit fields), so the original server's eligibility flags are
	// not readable from the actor. Harvesting is allowed for every such prop.
	bool IsLevelHarvestable(const ABuildingSMActor* Building)
	{
		return Building && !Building->bPlayerPlaced && Building->ResourceType.GetValue() != EFortResourceType::None
			&& !Building->BuildingResourceAmountOverride.RowName.IsNone();
	}

	UFortBuildingActorSet* EnsureBuildingAttributeSet(ABuildingActor* Building)
	{
		if (!Building)
		{
			return nullptr;
		}

		if (Building->BuildingAttributeSet)
		{
			return Building->BuildingAttributeSet;
		}

		// Original 0x141bc15e0 / 0x141bc4a90 / 0x141b990d0: props with Policy==Lazy spawn without ASC/BuildingAttributeSet
		// and materialize them on demand when health is modified or forced.
		if (!Building->AbilitySystemComponent)
		{
			UFortAbilitySystemComponent* ASC = NewObject<UFortAbilitySystemComponent>(Building, TEXT("AbilitySystemComponent"));
			if (ASC)
			{
				// Created only on the server after the actor was already replicated, so it must replicate as a dynamic subobject,
				// not as a stably named component (SetNetAddressable would make the client look for a component it never created).
				ASC->SetIsReplicated(true);
				ASC->RegisterComponent();
				ASC->InitAbilityActorInfo(Building, Building);
				// stand-in: suppress gameplay cue net-multicast serialization mismatch on unported context structs.
				ASC->bSuppressGameplayCues = true;
				Building->AbilitySystemComponent = ASC;
				Building->ReplicatedAbilitySystemComponent = ASC;
			}
		}

		UClass* SetClass = Building->BuildingAttributeSetClass.Get();
		if (!SetClass)
		{
			SetClass = UFortBuildingActorSet::StaticClass();
		}

		UFortBuildingActorSet* AttrSet = NewObject<UFortBuildingActorSet>(Building, SetClass, TEXT("BuildingAttributeSet"));
		if (AttrSet)
		{
			if (Building->AbilitySystemComponent)
			{
				Building->AbilitySystemComponent->AddAttributeSetSubobject(AttrSet);
			}
			Building->BuildingAttributeSet = AttrSet;
			Building->ReplicatedBuildingAttributeSet = AttrSet;
		}

		return AttrSet;
	}

	UFortResourceItemDefinition* ResolveResourceItemDefinition(EFortResourceType::Type ResourceType)
	{
		// Original 0x141e63190: UFortGameData maps ResourceType (0=Wood, 1=Stone, 2=Metal, 3=Permanite, 4=GoldCurrency).
		UFortAssetManager* Manager = GEngine ? Cast<UFortAssetManager>(GEngine->AssetManager) : nullptr;
		UFortGameData* GameData = Manager ? Manager->GameDataCommon : nullptr;
		if (GameData)
		{
			switch (ResourceType)
			{
			case EFortResourceType::Wood:
				if (UFortResourceItemDefinition* Def = GameData->WoodItemDefinition.LoadSynchronous()) return Def;
				break;
			case EFortResourceType::Stone:
				if (UFortResourceItemDefinition* Def = GameData->StoneItemDefinition.LoadSynchronous()) return Def;
				break;
			case EFortResourceType::Metal:
				if (UFortResourceItemDefinition* Def = GameData->MetalItemDefinition.LoadSynchronous()) return Def;
				break;
			case EFortResourceType::Permanite:
				if (UFortResourceItemDefinition* Def = GameData->PermaniteItemDefinition.LoadSynchronous()) return Def;
				break;
			case EFortResourceType::GoldCurrency:
				if (UFortResourceItemDefinition* Def = GameData->GoldCurrencyItemDefinition.LoadSynchronous()) return Def;
				break;
			default:
				return nullptr;
			}
		}

		// stand-in: direct package load fallback when UFortAssetManager::GameDataCommon is not populated yet.
		switch (ResourceType)
		{
		case EFortResourceType::Wood:
			return LoadObject<UFortResourceItemDefinition>(nullptr, WoodItemPath);
		case EFortResourceType::Stone:
			return LoadObject<UFortResourceItemDefinition>(nullptr, StoneItemPath);
		case EFortResourceType::Metal:
			return LoadObject<UFortResourceItemDefinition>(nullptr, MetalItemPath);
		default:
			return nullptr;
		}
	}

	float EvaluateResourcePoolTotal(const ABuildingSMActor* Building, bool& OutUsedCurve)
	{
		OutUsedCurve = false;
		if (!Building)
		{
			return 0.0f;
		}

		const FCurveTableRowHandle& Override = Building->BuildingResourceAmountOverride;
		const UCurveTable* CurveTable = Override.CurveTable;
		if (!CurveTable && !Override.RowName.IsNone())
		{
			CurveTable = LoadObject<UCurveTable>(nullptr, AthenaResourceRatesPath);
		}

		float Pool = 0.0f;
		if (CurveTable && !Override.RowName.IsNone())
		{
			// 12.60 AthenaResourceRates convention: Time=0 stores full-health base resource yield.
			if (const FRealCurve* Curve = CurveTable->FindCurve(Override.RowName, TEXT("BuildingResourceAmountOverride"), false))
			{
				Pool = Curve->Eval(0.0f);
				OutUsedCurve = true;
			}
		}

		if (Building->MaxResourcesToSpawn > 0)
		{
			Pool = FMath::Min(Pool, static_cast<float>(Building->MaxResourcesToSpawn));
		}

		return FMath::Max(0.0f, Pool);
	}

	void UpdateOrSpawnWeakSpot(
		AFortPlayerController* PC,
		ABuildingSMActor* Building,
		const FHitResult& Hit,
		bool bWeakSpotHit)
	{
		if (!PC || !Building || !(Building->bAllowWeakSpots || IsLevelHarvestable(Building)) || Building->bDestroyed)
		{
			return;
		}

		// Original 0x1423b5c20 / 0x142a45e10: tracks FBuildingWeakSpotData on AFortPlayerController::ActiveWeakSpots and calls ClientSpawnWeakSpotOnBuildingActor.
		int32 FoundIdx = INDEX_NONE;
		for (int32 i = 0; i < PC->ActiveWeakSpots.Num(); ++i)
		{
			if (PC->ActiveWeakSpots[i].ParentBuilding.Get() == Building)
			{
				FoundIdx = i;
				break;
			}
		}

		FBuildingWeakSpotData Data;
		Data.ParentBuilding = Building;
		const FVector ImpactNormal = Hit.ImpactNormal.IsNearlyZero() ? FVector::UpVector : FVector(Hit.ImpactNormal).GetSafeNormal();
		Data.Normal = FVector_NetQuantizeNormal(ImpactNormal);

		// binary 0x1423b5c20 (AFortPlayerController::SpawnWeakSpot): takes the player's view point (PC vtable +0x6f8), turns the
		// view rotation into a direction, and asks the building (vtable +0xab8) for the surface point where a 400 uu ray
		// from the view location along that direction meets it; that point and its normal become the weak spot. So the
		// circle sits where the player is looking at the object, not around the pickaxe hit.
		// stand-in: the building helper (vtable +0xab8) is not ported; ActorLineTraceSingle against the building plays its
		// role. The original keeps the first pitch on the entry (+0x24) for later spawns and randomizes inside the helper;
		// neither is recovered. If the ray misses, the pickaxe impact point is used as it is (no offset).
		const int32 NextHitCount = (FoundIdx != INDEX_NONE) ? (PC->ActiveWeakSpots[FoundIdx].HitCount + (bWeakSpotHit ? 1 : 0)) : 1;
		FVector BasePos = Hit.ImpactPoint.IsNearlyZero() ? Building->GetActorLocation() : FVector(Hit.ImpactPoint);
		FVector SpotNormal = ImpactNormal;
		{
			FVector ViewLocation;
			FRotator ViewRotation;
			PC->GetPlayerViewPoint(ViewLocation, ViewRotation);
			static const float RayLength = []() { float V = 400.0f; FParse::Value(FCommandLine::Get(), TEXT("FortniteWeakSpotRay="), V); return FMath::Max(50.0f, V); }();
			// The third-person camera sits behind the pawn: start the ray at the camera and reach past the pawn.
			const float CameraBack = FMath::Max(0.0f, FVector::Dist(ViewLocation, PC->GetPawn() ? PC->GetPawn()->GetActorLocation() : ViewLocation));
			const FVector RayEnd = ViewLocation + ViewRotation.Vector() * (RayLength + CameraBack);
			FHitResult RayHit;
			FCollisionQueryParams RayParams(FName(TEXT("WeakSpotRay")), true);
			if (Building->ActorLineTraceSingle(RayHit, ViewLocation, RayEnd, ECC_Visibility, RayParams))
			{
				BasePos = RayHit.ImpactPoint;
				SpotNormal = RayHit.ImpactNormal.GetSafeNormal();
			}
		}
		Data.Normal = FVector_NetQuantizeNormal(SpotNormal);
		Data.Position = FVector_NetQuantize10(BasePos);
		Data.HitCount = NextHitCount;

		if (FoundIdx != INDEX_NONE)
		{
			PC->ActiveWeakSpots[FoundIdx] = Data;
		}
		else
		{
			PC->ActiveWeakSpots.Add(Data);
		}

		PC->ClientSpawnWeakSpotOnBuildingActor(Data);
	}

	void ClearWeakSpotsForBuilding(AFortPlayerController* PC, ABuildingSMActor* Building)
	{
		if (!PC || !Building)
		{
			return;
		}

		for (int32 i = PC->ActiveWeakSpots.Num() - 1; i >= 0; --i)
		{
			if (PC->ActiveWeakSpots[i].ParentBuilding.Get() == Building || !PC->ActiveWeakSpots[i].ParentBuilding.IsValid())
			{
				PC->ActiveWeakSpots.RemoveAtSwap(i);
			}
		}
		PC->MulticastClearWeakSpotData(Building);
	}
} // namespace

bool IsEnabled()
{
	return FParse::Param(FCommandLine::Get(), TEXT("FortniteOriginalClient"));
}

void EnsureBuildingAttributeDataLoaded()
{
	(void)GetBuildingAttributeInitter();
}

float GetBuildingMaxHealth(ABuildingActor* Building)
{
	if (!Building)
	{
		return 0.0f;
	}

	// Original 0x141ba73d0: BuildingAttributeSet->MaxHealth if instantiated, else cached MaxHealthInitializationValue != -1.0f, else curve lookup.
	if (Building->BuildingAttributeSet)
	{
		const float AttrMax = Building->BuildingAttributeSet->MaxHealth.GetCurrentValue();
		if (AttrMax > 0.0f)
		{
			return AttrMax;
		}
	}

	if (Building->MaxHealthInitializationValue > 0.0f)
	{
		return Building->MaxHealthInitializationValue;
	}

	const float Resolved = ResolveMaxHealthFromCurves(Building);
	if (Resolved > 0.0f)
	{
		Building->MaxHealthInitializationValue = Resolved;
		return Resolved;
	}

	// stand-in: synthetic props lacking AttributeInitKeys fall back to 100.0f so health/resource math cannot divide by zero.
	return 100.0f;
}

float GetBuildingHealth(const ABuildingActor* Building)
{
	if (!Building)
	{
		return 0.0f;
	}

	if (Building->bDestroyed)
	{
		return 0.0f;
	}

	// Original 0x141b9edb0: reads BuildingAttributeSet->Health when allocated, else returns GetMaxHealth (undamaged lazy actor).
	if (Building->BuildingAttributeSet)
	{
		return FMath::Max(0.0f, Building->BuildingAttributeSet->Health.GetCurrentValue());
	}

	return GetBuildingMaxHealth(const_cast<ABuildingActor*>(Building));
}

bool InitializeBuildingHealth(ABuildingActor* Building)
{
	if (!Building)
	{
		return false;
	}

	// Initialize once per actor: re-running this on every hit reset Health back to full (damage never accumulated).
	static TSet<TWeakObjectPtr<ABuildingActor>> InitializedBuildings;
	if (InitializedBuildings.Contains(Building))
	{
		return true;
	}
	if (InitializedBuildings.Num() > 20000)
	{
		for (auto It = InitializedBuildings.CreateIterator(); It; ++It)
		{
			if (!It->IsValid())
			{
				It.RemoveCurrent();
			}
		}
	}

	EnsureBuildingAttributeDataLoaded();

	const float MaxHealth = GetBuildingMaxHealth(Building);
	if (MaxHealth <= 0.0f)
	{
		return false;
	}

	InitializedBuildings.Add(Building);
	Building->MaxHealthInitializationValue = MaxHealth;
	const float InitialPct = ResolveInitialHealthPct(Building);
	const float InitialHealth = FMath::Clamp(MaxHealth * InitialPct, 0.0f, MaxHealth);

	if (Building->BuildingAttributeSet)
	{
		Building->BuildingAttributeSet->MaxHealth.SetBaseValue(MaxHealth);
		Building->BuildingAttributeSet->MaxHealth.SetCurrentValue(MaxHealth);
		Building->BuildingAttributeSet->Health.SetBaseValue(InitialHealth);
		Building->BuildingAttributeSet->Health.SetCurrentValue(InitialHealth);
	}

	return true;
}

void ForceBuildingHealth(ABuildingActor* Building, float NewHealth)
{
	if (!Building)
	{
		return;
	}

	const float MaxHealth = FMath::Max(1.0f, GetBuildingMaxHealth(Building));
	const float ClampedHealth = FMath::Clamp(NewHealth, 0.0f, MaxHealth);

	if (UFortBuildingActorSet* AttrSet = EnsureBuildingAttributeSet(Building))
	{
		AttrSet->MaxHealth.SetBaseValue(MaxHealth);
		AttrSet->MaxHealth.SetCurrentValue(MaxHealth);
		AttrSet->Health.SetBaseValue(ClampedHealth);
		AttrSet->Health.SetCurrentValue(ClampedHealth);

		if (Building->AbilitySystemComponent)
		{
			Building->AbilitySystemComponent->SetNumericAttributeBase(GetMaxHealthAttribute(), MaxHealth);
			Building->AbilitySystemComponent->SetNumericAttributeBase(GetHealthAttribute(), ClampedHealth);
		}
	}

	Building->FlushNetDormancy(); // level props are DORM_Initial / 1 Hz: push the new health and attribute set to clients now
	Building->ForceNetUpdate();
}

bool ApplyBuildingDamage(
	ABuildingSMActor* Building,
	float Damage,
	AController* Instigator,
	AActor* Causer,
	const FHitResult& Hit,
	bool bWeakSpot)
{
	if (!IsEnabled())
	{
		return false;
	}

	if (!Building || !Building->IsValidLowLevel() || Building->bDestroyed || Damage <= 0.0f)
	{
		return false;
	}
	if (Building->bPlayerPlaced)
	{
		return FortAthenaBuildDamage::OnPlayerBuiltPieceDamaged(Building, Damage, Instigator, Causer, Hit, bWeakSpot);
	}

	if (Building->GetLocalRole() != ROLE_Authority)
	{
		return false;
	}

	InitializeBuildingHealth(Building);

	const float MaxHealth = FMath::Max(1.0f, GetBuildingMaxHealth(Building));
	const float OldHealth = GetBuildingHealth(Building);
	if (OldHealth <= 0.0f)
	{
		return false;
	}

	const float EffectiveDamage = Damage; // weak-spot multiplier is already in Damage (SurfaceDamageTeams AthenaWeakSpot, FortHarvestDamage.cpp)
	const float ClampedDamage = FMath::Min(EffectiveDamage, OldHealth);
	const float NewHealth = FMath::Max(0.0f, OldHealth - ClampedDamage);
	const bool bKilledThisHit = (NewHealth <= 0.0f);

	// 1. Update BuildingAttributeSet / ASC health (original 0x141bc4a90 / 0x141b990d0).
	ForceBuildingHealth(Building, NewHealth);

	// 2. Fire OnDamageServer BlueprintImplementableEvent and OnDamaged delegate (original 0x141baaf60 -> 0x142734dc0).
	FGameplayTagContainer DamageTags;
	FGameplayEffectContextHandle EffectContext;
	const FVector ImpactNormal = Hit.ImpactNormal.IsNearlyZero() ? FVector::UpVector : FVector(Hit.ImpactNormal).GetSafeNormal();
	const FVector Momentum = -ImpactNormal * ClampedDamage; // stand-in: synthesized momentum when caller does not supply GameplayEffectContext.
	FHitResult MutableHit = Hit;
	const FVector HitPoint = Hit.ImpactPoint.IsNearlyZero() ? Building->GetActorLocation() : FVector(Hit.ImpactPoint);

	Building->OnDamageServer(ClampedDamage, DamageTags, Momentum, MutableHit, Instigator, Causer, EffectContext);
	Building->OnDamaged.Broadcast(Building, ClampedDamage, Instigator, Causer, HitPoint, Hit.Component.Get(), Hit.BoneName, Momentum);

	// 3. Resource calculation, inventory grant (999 stack cap + pickup overflow), and client feedback RPC.
	AFortPlayerController* PC = Cast<AFortPlayerController>(Instigator);
	const EFortResourceType::Type ResType = Building->ResourceType.GetValue();
	if (PC && ResType != EFortResourceType::None && FortAthenaBuildDamage::ShouldAllowResourceDrop(Building) && (Building->bAllowResourceDrop || IsLevelHarvestable(Building))
		&& Building->BlueprintCanAttemptGenerateResources(DamageTags, Instigator))
	{
		bool bUsedCurve = false;
		const float ResourcePool = EvaluateResourcePoolTotal(Building, bUsedCurve);

		// stand-in: ABuildingSMActor::vtable[+0xb28] (0x1406801c0) and K2_GiveBuildingResource (0x1429732f0)
		// are stripped in the client binary. Compute cumulative fractional drop so hits across [MaxHealth -> 0] sum to RoundToInt(Pool).
		const int32 PrevTotal = FMath::FloorToInt(((MaxHealth - OldHealth) / MaxHealth) * ResourcePool);
		const int32 NextTotal = bKilledThisHit
			? FMath::RoundToInt(ResourcePool)
			: FMath::FloorToInt(((MaxHealth - NewHealth) / MaxHealth) * ResourcePool);
		const int32 GrantedAmount = FMath::Max(0, NextTotal - PrevTotal);
		UE_LOG(LogFortHarvestBuilding, Display, TEXT("Building: %s resource type=%d pool=%.1f curve=%d row=%s max=%d granted=%d (old=%.0f new=%.0f)"),
			*Building->GetName(), static_cast<int32>(ResType), ResourcePool, bUsedCurve ? 1 : 0,
			*Building->BuildingResourceAmountOverride.RowName.ToString(), Building->MaxResourcesToSpawn, GrantedAmount, OldHealth, NewHealth);

		if (GrantedAmount > 0)
		{
			if (UFortResourceItemDefinition* ResourceDef = ResolveResourceItemDefinition(ResType))
			{
				int32 AcceptedCount = 0;
				FortAthenaInventory::GiveItem(PC, ResourceDef, GrantedAmount, 1, 0, &AcceptedCount);

				// Community sources (FINDINGS.md section 2, community-measured) say harvested materials above the 999 cap are
				// wasted, not dropped as pickups. The original server body is stripped from the client binary.
				const int32 OverflowCount = FMath::Max(0, GrantedAmount - AcceptedCount);
				if (OverflowCount > 0)
				{
					UE_LOG(LogFortHarvestBuilding, Verbose, TEXT("Building: %d %s over the stack cap discarded"), OverflowCount, *ResourceDef->GetName());
				}

				PC->ClientReportDamagedResourceBuilding(Building, ResType, GrantedAmount, bKilledThisHit, bWeakSpot);
			}
			else
			{
				UE_LOG(LogFortHarvestBuilding, Warning, TEXT("Building: No resource item definition for type %d"), static_cast<int32>(ResType));
			}
		}
		else if (!bUsedCurve)
		{
			UE_LOG(LogFortHarvestBuilding, Verbose, TEXT("Building: %s had no BuildingResourceAmountOverride curve"), *Building->GetName());
		}
	}

	else if (PC)
	{
		UE_LOG(LogFortHarvestBuilding, Display, TEXT("Building: %s no resource grant (type=%d allowDrop=%d canGen=%d maxSpawn=%d)"),
			*Building->GetName(), static_cast<int32>(ResType), Building->bAllowResourceDrop ? 1 : 0,
			Building->BlueprintCanAttemptGenerateResources(DamageTags, Instigator) ? 1 : 0, Building->MaxResourcesToSpawn);
	}

	// 4. Weak spot progression or destruction cleanup.
	if (!bKilledThisHit)
	{
		UpdateOrSpawnWeakSpot(PC, Building, Hit, bWeakSpot);
		return true;
	}

	// 5. Destruction / death sequence (original 0x141c0c330 -> 0x141c00e60 -> 0x141bac110 -> 0x141c0be60 -> 0x141babc20).
	Building->bDestroyed = 1;
	Building->OnDeathServer(ClampedDamage, DamageTags, Momentum, MutableHit, Instigator, Causer, EffectContext);
	Building->OnDied.Broadcast(Building, ClampedDamage, Instigator, Causer, HitPoint, Hit.Component.Get(), Hit.BoneName, Momentum);

	ClearWeakSpotsForBuilding(PC, Building);

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

	return true;
}
} // namespace FortHarvest
