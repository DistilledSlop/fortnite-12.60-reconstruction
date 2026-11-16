// Blueprint-callable natives that the original FortniteGame module exposes and that cooked Blueprints call.
//
// The generated classes now declare these functions (porting/behavior snippets with a default return value), so the Blueprint
// loader resolves the import. This file rebinds each exec thunk to a real implementation built on the server-side systems of this
// module (inventory flow, loot transfer, harvest building health). Before the declarations existed the loader logged
// "Skipping import Function ..." and the engine skipped the call (no effect, default result).
//
// binary: the original bodies are not recovered; each function is a stand-in derived from the name, the signature and the Blueprint
// call sites. Only server-authoritative behaviour is implemented here; client-only/cosmetic natives are not touched.
#include "FortBlueprintNatives.h"
#include "CoreMinimal.h"
#include "UObject/Class.h"
#include "UObject/Stack.h"
#include "UObject/ScriptMacros.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "HAL/IConsoleManager.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"
#include "ABuildingActor.h"
#include "ABuildingContainer.h"
#include "AFortGameModeAthena.h"
#include "AFortInventory.h"
#include "AFortPawn.h"
#include "AFortPickup.h"
#include "AFortPlayerController.h"
#include "AFortPlayerPawn.h"
#include "AFortPlayerState.h"
#include "AFortPlayerStateAthena.h"
#include "AFortWeapon.h"
#include "EFortPickupSourceTypeFlag.h"
#include "EFortPickupSpawnSource.h"
#include "EFortTeamAffiliation.h"
#include "FFortItemEntry.h"
#include "UFortAbilitySystemBlueprintLibrary.h"
#include "UFortAbilitySystemComponent.h"
#include "UFortGlobals.h"
#include "UFortItem.h"
#include "UFortItemDefinition.h"
#include "UFortKismetLibrary.h"
#include "UFortWorldItem.h"
#include "UFortWorldItemDefinition.h"
#include "FortAthenaInventoryFlow.h"
#include "FortAthenaLoot.h"
#include "FortAthenaLootRoll.h"
#include "FortAthenaLootSearch.h"
#include "FortHarvestBuilding.h"

DEFINE_LOG_CATEGORY_STATIC(LogFortBpNatives, Log, All);

namespace
{
	constexpr uint8 NoTeam = 255;

	uint8 TeamOfActor(const AActor* Actor)
	{
		if (!Actor)
		{
			return NoTeam;
		}
		if (const AFortPlayerState* State = Cast<AFortPlayerState>(Actor))
		{
			return const_cast<AFortPlayerState*>(State)->NativeGetTeam163();
		}
		if (const AController* Controller = Cast<AController>(Actor))
		{
			return TeamOfActor(Controller->PlayerState);
		}
		if (const APawn* Pawn = Cast<APawn>(Actor))
		{
			return TeamOfActor(Pawn->GetPlayerState());
		}
		if (const ABuildingActor* Building = Cast<ABuildingActor>(Actor))
		{
			return Building->TeamIndex;
		}
		if (const AFortPlayerController* Owner = Cast<AFortPlayerController>(Actor->GetOwner()))
		{
			return TeamOfActor(Owner);
		}
		return NoTeam;
	}

	AFortPlayerController* ControllerFromActor(AActor* Actor)
	{
		if (!Actor)
		{
			return nullptr;
		}
		if (AFortPlayerController* Controller = Cast<AFortPlayerController>(Actor))
		{
			return Controller;
		}
		if (APawn* Pawn = Cast<APawn>(Actor))
		{
			return Cast<AFortPlayerController>(Pawn->GetController());
		}
		if (APlayerState* State = Cast<APlayerState>(Actor))
		{
			return Cast<AFortPlayerController>(State->GetOwner());
		}
		return Cast<AFortPlayerController>(Actor->GetOwner());
	}

	UWorld* WorldOf(UObject* WorldContextObject)
	{
		return WorldContextObject ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	}

	void FindItemsByDefinition(AFortPlayerController* Controller, const UFortItemDefinition* Definition, TArray<UFortWorldItem*>& Out)
	{
		AFortInventory* Inventory = Controller ? Controller->WorldInventory : nullptr;
		if (!Inventory || !Definition)
		{
			return;
		}
		for (UFortWorldItem* Item : Inventory->Inventory.ItemInstances)
		{
			if (Item && Item->ItemEntry.ItemDefinition == Definition)
			{
				Out.Add(Item);
			}
		}
	}

	int32 RemoveFromItems(AFortPlayerController* Controller, const TArray<UFortWorldItem*>& Items, int32 Amount)
	{
		// Amount < 0 removes every matching item completely.
		int32 Removed = 0;
		for (UFortWorldItem* Item : Items)
		{
			if (Amount >= 0 && Removed >= Amount)
			{
				break;
			}
			const int32 Have = Item->ItemEntry.Count;
			const int32 Take = Amount < 0 ? Have : FMath::Min(Have, Amount - Removed);
			if (Take > 0)
			{
				Removed += FortAthenaInventory::RemoveItem(Controller, Item->NativeGetItemGuid(), Take);
			}
		}
		return Removed;
	}

	// ---- UFortKismetLibrary ------------------------------------------------------------------------------------------------

	DEFINE_FUNCTION(ExecGetActorTeam)
	{
		P_GET_OBJECT(AActor, Actor);
		P_FINISH;
		P_NATIVE_BEGIN;
		*static_cast<uint8*>(RESULT_PARAM) = TeamOfActor(Actor);
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecOnSameTeam)
	{
		P_GET_OBJECT(AActor, ActorA);
		P_GET_OBJECT(AActor, ActorB);
		P_FINISH;
		P_NATIVE_BEGIN;
		const uint8 TeamA = TeamOfActor(ActorA);
		*static_cast<bool*>(RESULT_PARAM) = TeamA != NoTeam && TeamA == TeamOfActor(ActorB);
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecGetTeamAffiliation)
	{
		P_GET_OBJECT(AActor, ActorA);
		P_GET_OBJECT(AActor, ActorB);
		P_FINISH;
		P_NATIVE_BEGIN;
		const uint8 TeamA = TeamOfActor(ActorA);
		const uint8 TeamB = TeamOfActor(ActorB);
		EFortTeamAffiliation::Type Result = EFortTeamAffiliation::Neutral;
		if (TeamA != NoTeam && TeamB != NoTeam)
		{
			Result = TeamA == TeamB ? EFortTeamAffiliation::Friendly : EFortTeamAffiliation::Hostile;
		}
		*static_cast<uint8*>(RESULT_PARAM) = static_cast<uint8>(Result);
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecGetFortPlayerControllerFromActor)
	{
		P_GET_OBJECT(AActor, Actor);
		P_FINISH;
		P_NATIVE_BEGIN;
		*static_cast<AFortPlayerController**>(RESULT_PARAM) = ControllerFromActor(Actor);
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecGetGameStateAthena)
	{
		P_GET_OBJECT(UObject, WorldContextObject);
		P_FINISH;
		P_NATIVE_BEGIN;
		UWorld* World = WorldOf(WorldContextObject);
		*static_cast<AFortGameStateAthena**>(RESULT_PARAM) = World ? Cast<AFortGameStateAthena>(World->GetGameState()) : nullptr;
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecGetServerWorldTimeSeconds)
	{
		P_GET_OBJECT(UObject, WorldContextObject);
		P_FINISH;
		P_NATIVE_BEGIN;
		UWorld* World = WorldOf(WorldContextObject);
		const AGameStateBase* State = World ? World->GetGameState() : nullptr;
		*static_cast<float*>(RESULT_PARAM) = State ? State->GetServerWorldTimeSeconds() : (World ? World->GetTimeSeconds() : 0.0f);
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecGetClosestActorFromArray)
	{
		P_GET_OBJECT(AActor, Actor);
		P_GET_TARRAY_REF(AActor*, ArrayOfActors);
		P_FINISH;
		P_NATIVE_BEGIN;
		AActor* Best = nullptr;
		if (Actor)
		{
			float BestDistSq = TNumericLimits<float>::Max();
			for (AActor* Candidate : ArrayOfActors)
			{
				if (Candidate && Candidate != Actor)
				{
					const float DistSq = FVector::DistSquared(Candidate->GetActorLocation(), Actor->GetActorLocation());
					if (DistSq < BestDistSq)
					{
						BestDistSq = DistSq;
						Best = Candidate;
					}
				}
			}
		}
		*static_cast<AActor**>(RESULT_PARAM) = Best;
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecCheckLineOfSightToActor)
	{
		P_GET_STRUCT_REF(FVector, SourcePos);
		P_GET_OBJECT(AActor, Target);
		P_GET_OBJECT(AActor, Source);
		P_FINISH;
		P_NATIVE_BEGIN;
		bool bVisible = false;
		if (Target && Target->GetWorld())
		{
			FCollisionQueryParams Params(SCENE_QUERY_STAT(FortLineOfSight), false, Source);
			Params.AddIgnoredActor(Target);
			FHitResult Hit;
			bVisible = !Target->GetWorld()->LineTraceSingleByChannel(Hit, SourcePos, Target->GetActorLocation(), ECC_Visibility, Params);
		}
		*static_cast<bool*>(RESULT_PARAM) = bVisible;
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecSetCanBeDamaged)
	{
		P_GET_OBJECT(AActor, Actor);
		P_GET_UBOOL(bCanBeDamaged);
		P_FINISH;
		P_NATIVE_BEGIN;
		if (Actor)
		{
			Actor->SetCanBeDamaged(bCanBeDamaged);
		}
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecGiveItemToInventoryOwner)
	{
		P_GET_STRUCT(FScriptInterface, InventoryOwner);
		P_GET_OBJECT(UFortWorldItemDefinition, ItemDefinition);
		P_GET_PROPERTY(FIntProperty, NumberToGive);
		P_GET_UBOOL(bNotifyPlayer);
		P_GET_PROPERTY(FIntProperty, ItemLevel);
		P_GET_PROPERTY(FIntProperty, PickupInstigatorHandle);
		P_FINISH;
		P_NATIVE_BEGIN;
		AFortPlayerController* Controller = ControllerFromActor(Cast<AActor>(InventoryOwner.GetObject()));
		if (Controller && ItemDefinition && NumberToGive > 0)
		{
			FortAthenaInventory::GiveItem(Controller, ItemDefinition, NumberToGive, FMath::Max(ItemLevel, 1));
		}
		else
		{
			UE_LOG(LogFortBpNatives, Warning, TEXT("GiveItemToInventoryOwner: ignored (owner=%s definition=%s count=%d)"),
				*GetNameSafe(InventoryOwner.GetObject()), *GetNameSafe(ItemDefinition), NumberToGive);
		}
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecK2_RemoveItemFromPlayer)
	{
		P_GET_OBJECT(AFortPlayerController, PlayerController);
		P_GET_OBJECT(UFortWorldItemDefinition, ItemDefinition);
		P_GET_PROPERTY(FIntProperty, AmountToRemove);
		P_GET_UBOOL(bForceRemoval);
		P_FINISH;
		P_NATIVE_BEGIN;
		TArray<UFortWorldItem*> Items;
		FindItemsByDefinition(PlayerController, ItemDefinition, Items);
		*static_cast<int32*>(RESULT_PARAM) = RemoveFromItems(PlayerController, Items, AmountToRemove);
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecK2_RemoveFortItemFromPlayer)
	{
		P_GET_OBJECT(AFortPlayerController, PlayerController);
		P_GET_OBJECT(UFortItem, Item);
		P_GET_PROPERTY(FIntProperty, AmountToRemove);
		P_GET_UBOOL(bForceRemoval);
		P_FINISH;
		P_NATIVE_BEGIN;
		TArray<UFortWorldItem*> Items;
		if (UFortWorldItem* WorldItem = Cast<UFortWorldItem>(Item))
		{
			Items.Add(WorldItem);
		}
		*static_cast<int32*>(RESULT_PARAM) = RemoveFromItems(PlayerController, Items, AmountToRemove);
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecK2_GetItemQuantityOnPlayer)
	{
		P_GET_OBJECT(AFortPlayerController, PlayerController);
		P_GET_OBJECT(UFortItemDefinition, ItemDefinition);
		P_FINISH;
		P_NATIVE_BEGIN;
		TArray<UFortWorldItem*> Items;
		FindItemsByDefinition(PlayerController, ItemDefinition, Items);
		int32 Total = 0;
		for (const UFortWorldItem* Item : Items)
		{
			Total += Item->ItemEntry.Count;
		}
		*static_cast<int32*>(RESULT_PARAM) = Total;
		P_NATIVE_END;
	}

	AFortPickup* SpawnPickupCommon(UObject* WorldContextObject, UFortWorldItemDefinition* ItemDefinition, int32 NumberToSpawn,
		const FVector& Position, bool bToss)
	{
		UWorld* World = WorldOf(WorldContextObject);
		if (!World || !ItemDefinition || NumberToSpawn <= 0)
		{
			return nullptr;
		}
		const FVector TossFrom = Position;
		return FortAthenaLoot::SpawnResolvedPickup(World, Position, ItemDefinition, NumberToSpawn, 1, 0, bToss ? &TossFrom : nullptr);
	}

	DEFINE_FUNCTION(ExecK2_SpawnPickupInWorld)
	{
		P_GET_OBJECT(UObject, WorldContextObject);
		P_GET_OBJECT(UFortWorldItemDefinition, ItemDefinition);
		P_GET_PROPERTY(FIntProperty, NumberToSpawn);
		P_GET_STRUCT(FVector, Position);
		P_GET_STRUCT(FVector, Direction);
		P_GET_PROPERTY(FIntProperty, OverrideMaxStackCount);
		P_GET_UBOOL(bToss);
		P_GET_UBOOL(bRandomRotation);
		P_GET_UBOOL(bBlockedFromAutoPickup);
		P_GET_PROPERTY(FIntProperty, PickupInstigatorHandle);
		P_GET_ENUM(EFortPickupSourceTypeFlag, SourceType);
		P_GET_ENUM(EFortPickupSpawnSource, Source);
		P_GET_OBJECT(AFortPlayerController, OptionalOwnerPC);
		P_GET_UBOOL(bPickupOnlyRelevantToOwner);
		P_FINISH;
		P_NATIVE_BEGIN;
		*static_cast<AFortPickup**>(RESULT_PARAM) = SpawnPickupCommon(WorldContextObject, ItemDefinition, NumberToSpawn, Position, bToss);
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecK2_SpawnPickupInWorldWithClass)
	{
		P_GET_OBJECT(UObject, WorldContextObject);
		P_GET_OBJECT(UFortWorldItemDefinition, ItemDefinition);
		P_GET_OBJECT(UClass, PickupClass);
		P_GET_PROPERTY(FIntProperty, NumberToSpawn);
		P_GET_STRUCT(FVector, Position);
		P_GET_STRUCT(FVector, Direction);
		P_GET_PROPERTY(FIntProperty, OverrideMaxStackCount);
		P_GET_UBOOL(bToss);
		P_GET_UBOOL(bRandomRotation);
		P_GET_UBOOL(bBlockedFromAutoPickup);
		P_GET_PROPERTY(FIntProperty, PickupInstigatorHandle);
		P_GET_ENUM(EFortPickupSourceTypeFlag, SourceType);
		P_GET_ENUM(EFortPickupSpawnSource, Source);
		P_GET_OBJECT(AFortPlayerController, OptionalOwnerPC);
		P_GET_UBOOL(bPickupOnlyRelevantToOwner);
		P_FINISH;
		P_NATIVE_BEGIN;
		// stand-in: the pickup class override is ignored; the loot system spawns FortPickupAthena.
		*static_cast<AFortPickup**>(RESULT_PARAM) = SpawnPickupCommon(WorldContextObject, ItemDefinition, NumberToSpawn, Position, bToss);
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecCreateItemEntry)
	{
		P_GET_OBJECT(UFortItemDefinition, InItemDefinition);
		P_GET_PROPERTY(FIntProperty, InCount);
		P_GET_PROPERTY(FIntProperty, InLevel);
		P_FINISH;
		P_NATIVE_BEGIN;
		FFortItemEntry Entry;
		Entry.ItemDefinition = InItemDefinition;
		Entry.Count = InCount;
		Entry.Level = InLevel;
		Entry.ItemGuid = FGuid::NewGuid();
		*static_cast<FFortItemEntry*>(RESULT_PARAM) = Entry;
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecGetBuildingHealthPercentage)
	{
		P_GET_OBJECT(ABuildingActor, BuildingActor);
		P_FINISH;
		P_NATIVE_BEGIN;
		float Percent = 0.0f;
		if (BuildingActor)
		{
			const float Max = FortHarvest::GetBuildingMaxHealth(BuildingActor);
			Percent = Max > 0.0f ? FortHarvest::GetBuildingHealth(BuildingActor) / Max : 0.0f;
		}
		*static_cast<float*>(RESULT_PARAM) = Percent;
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecGetNumericCVar)
	{
		P_GET_PROPERTY(FStrProperty, CVar);
		P_GET_PROPERTY_REF(FFloatProperty, OutValue);
		P_FINISH;
		P_NATIVE_BEGIN;
		bool bFound = false;
		if (IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(*CVar))
		{
			OutValue = Variable->GetFloat();
			bFound = true;
		}
		*static_cast<bool*>(RESULT_PARAM) = bFound;
		P_NATIVE_END;
	}

	// ---- UFortGlobals ------------------------------------------------------------------------------------------------------

	DEFINE_FUNCTION(ExecIsInAthena)
	{
		P_GET_OBJECT(UObject, WorldContextObject);
		P_FINISH;
		P_NATIVE_BEGIN;
		UWorld* World = WorldOf(WorldContextObject);
		*static_cast<bool*>(RESULT_PARAM) = World && Cast<AFortGameModeAthena>(World->GetAuthGameMode()) != nullptr;
		P_NATIVE_END;
	}

	// ---- Ability system ----------------------------------------------------------------------------------------------------

	DEFINE_FUNCTION(ExecGetFortAbilitySystemComponent)
	{
		P_GET_OBJECT(AActor, Actor);
		P_FINISH;
		P_NATIVE_BEGIN;
		UFortAbilitySystemComponent* Result = nullptr;
		if (const IAbilitySystemInterface* Interface = Cast<IAbilitySystemInterface>(Actor))
		{
			Result = Cast<UFortAbilitySystemComponent>(Interface->GetAbilitySystemComponent());
		}
		*static_cast<UFortAbilitySystemComponent**>(RESULT_PARAM) = Result;
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecMakeFortEffectContext)
	{
		P_GET_OBJECT(UObject, SourceObject);
		P_GET_OBJECT(AActor, EffectCauser);
		P_GET_OBJECT(AActor, DamageSource);
		P_GET_PROPERTY(FIntProperty, GameplayEffectLevel);
		P_FINISH;
		P_NATIVE_BEGIN;
		FGameplayEffectContextHandle Handle;
		if (UAbilitySystemComponent* ASC = Cast<UAbilitySystemComponent>(Context))
		{
			Handle = ASC->MakeEffectContext();
			if (SourceObject)
			{
				Handle.AddSourceObject(SourceObject);
			}
			if (EffectCauser)
			{
				Handle.AddInstigator(ASC->GetAvatarActor(), EffectCauser);
			}
		}
		*static_cast<FGameplayEffectContextHandle*>(RESULT_PARAM) = Handle;
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecBP_FortApplyGameplayEffectToTarget)
	{
		P_GET_OBJECT(UClass, GameplayEffectClass);
		P_GET_OBJECT(UAbilitySystemComponent, TargetOfEffect);
		P_GET_OBJECT(UObject, OptionalSourceObject);
		P_GET_OBJECT(AActor, OptionalEffectCauser);
		P_GET_OBJECT(AActor, OptionalDamageSource);
		P_GET_PROPERTY(FFloatProperty, Level);
		P_GET_STRUCT(FGameplayEffectContextHandle, EffectContext);
		P_FINISH;
		P_NATIVE_BEGIN;
		FActiveGameplayEffectHandle Handle;
		UAbilitySystemComponent* Source = Cast<UAbilitySystemComponent>(Context);
		if (Source && TargetOfEffect && GameplayEffectClass && GameplayEffectClass->IsChildOf(UGameplayEffect::StaticClass()))
		{
			if (!EffectContext.IsValid())
			{
				EffectContext = Source->MakeEffectContext();
				if (OptionalSourceObject)
				{
					EffectContext.AddSourceObject(OptionalSourceObject);
				}
				if (OptionalEffectCauser)
				{
					EffectContext.AddInstigator(Source->GetAvatarActor(), OptionalEffectCauser);
				}
			}
			Handle = Source->BP_ApplyGameplayEffectToTarget(GameplayEffectClass, TargetOfEffect, Level, EffectContext);
		}
		*static_cast<FActiveGameplayEffectHandle*>(RESULT_PARAM) = Handle;
		P_NATIVE_END;
	}

	// ---- Actors ------------------------------------------------------------------------------------------------------------

	DEFINE_FUNCTION(ExecBP_FindItemInstancesFromDefinition)
	{
		P_GET_OBJECT(UFortItemDefinition, ItemDefinition);
		P_GET_TARRAY_REF(UFortItem*, ItemArray);
		P_FINISH;
		P_NATIVE_BEGIN;
		TArray<UFortWorldItem*> Items;
		FindItemsByDefinition(Cast<AFortPlayerController>(Context), ItemDefinition, Items);
		for (UFortWorldItem* Item : Items)
		{
			ItemArray.Add(Item);
		}
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecBP_FindExistingItemForDefinition)
	{
		P_GET_OBJECT(UFortItemDefinition, ItemDefinition);
		P_GET_UBOOL(bInStorageVault);
		P_FINISH;
		P_NATIVE_BEGIN;
		TArray<UFortWorldItem*> Items;
		FindItemsByDefinition(Cast<AFortPlayerController>(Context), ItemDefinition, Items);
		*static_cast<UFortItem**>(RESULT_PARAM) = Items.Num() > 0 ? Items[0] : nullptr;
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecBP_GetInventoryItemWithGuid)
	{
		P_GET_STRUCT(FGuid, ItemGuid);
		P_FINISH;
		P_NATIVE_BEGIN;
		UFortItem* Found = nullptr;
		const AFortPlayerController* Controller = Cast<AFortPlayerController>(Context);
		if (const AFortInventory* Inventory = Controller ? Controller->WorldInventory : nullptr)
		{
			for (UFortWorldItem* Item : Inventory->Inventory.ItemInstances)
			{
				if (Item && Item->ItemEntry.ItemGuid == ItemGuid)
				{
					Found = Item;
					break;
				}
			}
		}
		*static_cast<UFortItem**>(RESULT_PARAM) = Found;
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecHasHealthLeft)
	{
		P_FINISH;
		P_NATIVE_BEGIN;
		const ABuildingActor* Building = Cast<ABuildingActor>(Context);
		*static_cast<bool*>(RESULT_PARAM) = Building && FortHarvest::GetBuildingHealth(Building) > 0.0f;
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecSetHealth)
	{
		P_GET_PROPERTY(FFloatProperty, NewHealth);
		P_FINISH;
		P_NATIVE_BEGIN;
		if (ABuildingActor* Building = Cast<ABuildingActor>(Context))
		{
			FortHarvest::ForceBuildingHealth(Building, NewHealth);
		}
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecSetSearchLootTierGroup)
	{
		P_GET_PROPERTY(FNameProperty, InSearchLootTierGroup);
		P_FINISH;
		P_NATIVE_BEGIN;
		if (ABuildingContainer* Container = Cast<ABuildingContainer>(Context))
		{
			Container->SearchLootTierGroup = InSearchLootTierGroup;
		}
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecGetItemEntry)
	{
		P_GET_PROPERTY(FIntProperty, ItemIndex);
		P_FINISH;
		P_NATIVE_BEGIN;
		FFortItemEntry Entry;
		if (const AFortPickup* Pickup = Cast<AFortPickup>(Context))
		{
			Entry = Pickup->MultiItemPickupEntries.IsValidIndex(ItemIndex) ? Pickup->MultiItemPickupEntries[ItemIndex] : Pickup->PrimaryPickupItemEntry;
		}
		*static_cast<FFortItemEntry*>(RESULT_PARAM) = Entry;
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecGetPlayerPawnOrVehicleDriver)
	{
		P_FINISH;
		P_NATIVE_BEGIN;
		const APlayerState* State = Cast<APlayerState>(Context);
		*static_cast<AFortPlayerPawn**>(RESULT_PARAM) = State ? Cast<AFortPlayerPawn>(State->GetPawn()) : nullptr;
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecWeaponDataIsValid)
	{
		P_FINISH;
		P_NATIVE_BEGIN;
		const AFortWeapon* Weapon = Cast<AFortWeapon>(Context);
		*static_cast<bool*>(RESULT_PARAM) = Weapon && Weapon->WeaponData != nullptr;
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecPickLootDrops)
	{
		P_GET_OBJECT(UObject, WorldContextObject);
		P_GET_TARRAY_REF(FFortItemEntry, OutLootToDrop);
		P_GET_PROPERTY(FNameProperty, TierGroupName);
		P_GET_PROPERTY(FIntProperty, WorldLevel);
		P_GET_PROPERTY(FIntProperty, ForcedLootTier);
		P_FINISH;
		P_NATIVE_BEGIN;
		bool bOk = false;
		UWorld* World = WorldOf(WorldContextObject);
		const AFortGameModeAthena* Mode = World ? Cast<AFortGameModeAthena>(World->GetAuthGameMode()) : nullptr;
		if (Mode)
		{
			FortAthenaLootRoll::FRequest Request;
			Request.Group = TierGroupName;
			Request.WorldLevel = WorldLevel;
			Request.LootTier = ForcedLootTier >= 0 ? ForcedLootTier : -1;
			TArray<FortAthenaLootRoll::FDrop> Drops;
			FString Error;
			// stand-in: item levels are taken from the roll's loot level without the LootLevel curve table.
			bOk = FortAthenaLootRoll::RollForMode(Mode, Request, []() { return static_cast<double>(FMath::FRand()); },
				[](UFortItemDefinition*, int32 LootLevel, int32& OutLevel) { OutLevel = FMath::Max(LootLevel, 1); return true; }, Drops, Error);
			if (bOk)
			{
				for (const FortAthenaLootRoll::FDrop& Drop : Drops)
				{
					if (Drop.Definition && Drop.Count > 0)
					{
						FFortItemEntry Entry{};
						Entry.ItemDefinition = Drop.Definition;
						Entry.Count = Drop.Count;
						Entry.Level = Drop.Level;
						Entry.ItemGuid = FGuid::NewGuid();
						OutLootToDrop.Add(Entry);
					}
				}
			}
			else
			{
				UE_LOG(LogFortBpNatives, Warning, TEXT("PickLootDrops: group=%s roll failed: %s"), *TierGroupName.ToString(), *Error);
			}
		}
		*static_cast<bool*>(RESULT_PARAM) = bOk && OutLootToDrop.Num() > 0;
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecBP_SpawnLoot)
	{
		P_GET_OBJECT(AFortPlayerPawn, PlayerPawn);
		P_FINISH;
		P_NATIVE_BEGIN;
		ABuildingContainer* Container = Cast<ABuildingContainer>(Context);
		AFortPlayerController* Controller = PlayerPawn ? Cast<AFortPlayerController>(PlayerPawn->GetController()) : nullptr;
		*static_cast<bool*>(RESULT_PARAM) = Container && Controller && FortAthenaLootSearch::TrySearchContainer(Controller, Container);
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecK2_ConsumeAmmo)
	{
		P_GET_PROPERTY(FIntProperty, Quantity);
		P_FINISH;
		P_NATIVE_BEGIN;
		if (AFortWeapon* Weapon = Cast<AFortWeapon>(Context))
		{
			Weapon->AmmoCount = FMath::Max(0, Weapon->AmmoCount - FMath::Max(Quantity, 0));
			Weapon->ForceNetUpdate();
		}
		P_NATIVE_END;
	}

	// ---- stubbed server RPCs ---------------------------------------------------------------------------------------------

	// ServerCombineInventoryItems: empty in the retail client (stripped server body, shared stub 0x1406801c0).
	DEFINE_FUNCTION(ExecServerCombineInventoryItems)
	{
		P_GET_STRUCT_REF(FGuid, TargetItemGuid);
		P_GET_STRUCT_REF(FGuid, SourceItemGuid);
		P_FINISH;
		P_NATIVE_BEGIN;
		if (AFortPlayerController* Controller = Cast<AFortPlayerController>(Context))
		{
			FortAthenaInventory::CombineStacks(Controller, TargetItemGuid, SourceItemGuid);
		}
		P_NATIVE_END;
	}

	// ServerSpotActor: empty in the retail client. The spotting UI marks the actor locally; nothing server side is replicated here.
	DEFINE_FUNCTION(ExecServerSpotActor)
	{
		P_GET_OBJECT(AActor, NewlySpottedActor);
		P_FINISH;
		P_NATIVE_BEGIN;
		UE_LOG(LogFortBpNatives, Verbose, TEXT("ServerSpotActor: %s spotted %s"), *GetNameSafe(Context), *GetNameSafe(NewlySpottedActor));
		P_NATIVE_END;
	}

	// AFortWeapon::ServerReleaseWeaponAbility: the client's fire-button release for a weapon ability (the 21 calls in the 2026-11-15 run).
	// binary: implementation 0x1426b0cb0 -> 0x1426aec00 (read in porting/server_scope/local_model/claude/deps): looks up the spec on the
	// owner controller's ability system component and calls AbilitySpecInputReleased; the BP weapon-released notification only runs on a
	// local controller and is omitted on the server.
	DEFINE_FUNCTION(ExecServerReleaseWeaponAbility)
	{
		P_GET_STRUCT_REF(FGameplayAbilitySpecHandle, SpecHandle);
		P_FINISH;
		P_NATIVE_BEGIN;
		const AFortWeapon* Weapon = Cast<AFortWeapon>(Context);
		AFortPlayerController* Owner = Weapon ? Cast<AFortPlayerController>(Weapon->GetOwner()) : nullptr;
		const IAbilitySystemInterface* Interface = Owner ? Cast<IAbilitySystemInterface>(Owner->PlayerState) : nullptr;
		UAbilitySystemComponent* ASC = Interface ? Interface->GetAbilitySystemComponent() : nullptr;
		if (ASC && SpecHandle.IsValid())
		{
			if (FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(SpecHandle))
			{
				ASC->AbilitySpecInputReleased(*Spec);
			}
		}
		P_NATIVE_END;
	}

	// ServerSetAutoEquipBetterItems: stand-in that stores the client preference in the replicated flag (the retail body 0x1423ac470 was not read).
	DEFINE_FUNCTION(ExecServerSetAutoEquipBetterItems)
	{
		P_GET_UBOOL(bAutoEquip);
		P_FINISH;
		P_NATIVE_BEGIN;
		if (AFortPlayerController* Controller = Cast<AFortPlayerController>(Context))
		{
			Controller->bAutoEquipBetterItems = bAutoEquip;
		}
		P_NATIVE_END;
	}

	struct FNativeBinding
	{
		const TCHAR* ClassPath;
		const TCHAR* FunctionName;
		FNativeFuncPtr Exec;
	};

	const FNativeBinding GBindings[] = {
		{ TEXT("/Script/FortniteGame.FortKismetLibrary"), TEXT("GetActorTeam"), &ExecGetActorTeam },
		{ TEXT("/Script/FortniteGame.FortKismetLibrary"), TEXT("OnSameTeam"), &ExecOnSameTeam },
		{ TEXT("/Script/FortniteGame.FortKismetLibrary"), TEXT("GetTeamAffiliation"), &ExecGetTeamAffiliation },
		{ TEXT("/Script/FortniteGame.FortKismetLibrary"), TEXT("GetFortPlayerControllerFromActor"), &ExecGetFortPlayerControllerFromActor },
		{ TEXT("/Script/FortniteGame.FortKismetLibrary"), TEXT("GetGameStateAthena"), &ExecGetGameStateAthena },
		{ TEXT("/Script/FortniteGame.FortKismetLibrary"), TEXT("GetServerWorldTimeSeconds"), &ExecGetServerWorldTimeSeconds },
		{ TEXT("/Script/FortniteGame.FortKismetLibrary"), TEXT("GetClosestActorFromArray"), &ExecGetClosestActorFromArray },
		{ TEXT("/Script/FortniteGame.FortKismetLibrary"), TEXT("CheckLineOfSightToActor"), &ExecCheckLineOfSightToActor },
		{ TEXT("/Script/FortniteGame.FortKismetLibrary"), TEXT("SetCanBeDamaged"), &ExecSetCanBeDamaged },
		{ TEXT("/Script/FortniteGame.FortKismetLibrary"), TEXT("GiveItemToInventoryOwner"), &ExecGiveItemToInventoryOwner },
		{ TEXT("/Script/FortniteGame.FortKismetLibrary"), TEXT("K2_RemoveItemFromPlayer"), &ExecK2_RemoveItemFromPlayer },
		{ TEXT("/Script/FortniteGame.FortKismetLibrary"), TEXT("K2_RemoveFortItemFromPlayer"), &ExecK2_RemoveFortItemFromPlayer },
		{ TEXT("/Script/FortniteGame.FortKismetLibrary"), TEXT("K2_GetItemQuantityOnPlayer"), &ExecK2_GetItemQuantityOnPlayer },
		{ TEXT("/Script/FortniteGame.FortKismetLibrary"), TEXT("K2_SpawnPickupInWorld"), &ExecK2_SpawnPickupInWorld },
		{ TEXT("/Script/FortniteGame.FortKismetLibrary"), TEXT("K2_SpawnPickupInWorldWithClass"), &ExecK2_SpawnPickupInWorldWithClass },
		{ TEXT("/Script/FortniteGame.FortKismetLibrary"), TEXT("CreateItemEntry"), &ExecCreateItemEntry },
		{ TEXT("/Script/FortniteGame.FortKismetLibrary"), TEXT("GetBuildingHealthPercentage"), &ExecGetBuildingHealthPercentage },
		{ TEXT("/Script/FortniteGame.FortKismetLibrary"), TEXT("GetNumericCVar"), &ExecGetNumericCVar },
		{ TEXT("/Script/FortniteGame.FortGlobals"), TEXT("IsInAthena"), &ExecIsInAthena },
		{ TEXT("/Script/FortniteGame.FortAbilitySystemBlueprintLibrary"), TEXT("GetFortAbilitySystemComponent"), &ExecGetFortAbilitySystemComponent },
		{ TEXT("/Script/FortniteGame.FortAbilitySystemComponent"), TEXT("MakeFortEffectContext"), &ExecMakeFortEffectContext },
		{ TEXT("/Script/FortniteGame.FortAbilitySystemComponent"), TEXT("BP_FortApplyGameplayEffectToTarget"), &ExecBP_FortApplyGameplayEffectToTarget },
		{ TEXT("/Script/FortniteGame.FortPlayerController"), TEXT("BP_FindItemInstancesFromDefinition"), &ExecBP_FindItemInstancesFromDefinition },
		{ TEXT("/Script/FortniteGame.FortPlayerController"), TEXT("BP_FindExistingItemForDefinition"), &ExecBP_FindExistingItemForDefinition },
		{ TEXT("/Script/FortniteGame.FortPlayerController"), TEXT("BP_GetInventoryItemWithGuid"), &ExecBP_GetInventoryItemWithGuid },
		{ TEXT("/Script/FortniteGame.BuildingActor"), TEXT("HasHealthLeft"), &ExecHasHealthLeft },
		{ TEXT("/Script/FortniteGame.BuildingActor"), TEXT("SetHealth"), &ExecSetHealth },
		{ TEXT("/Script/FortniteGame.BuildingContainer"), TEXT("SetSearchLootTierGroup"), &ExecSetSearchLootTierGroup },
		{ TEXT("/Script/FortniteGame.FortPickup"), TEXT("GetItemEntry"), &ExecGetItemEntry },
		{ TEXT("/Script/FortniteGame.FortPlayerState"), TEXT("GetPlayerPawnOrVehicleDriver"), &ExecGetPlayerPawnOrVehicleDriver },
		{ TEXT("/Script/FortniteGame.FortWeapon"), TEXT("WeaponDataIsValid"), &ExecWeaponDataIsValid },
		{ TEXT("/Script/FortniteGame.FortKismetLibrary"), TEXT("PickLootDrops"), &ExecPickLootDrops },
		{ TEXT("/Script/FortniteGame.BuildingContainer"), TEXT("BP_SpawnLoot"), &ExecBP_SpawnLoot },
		{ TEXT("/Script/FortniteGame.FortWeapon"), TEXT("K2_ConsumeAmmo"), &ExecK2_ConsumeAmmo },
		{ TEXT("/Script/FortniteGame.FortPlayerController"), TEXT("ServerCombineInventoryItems"), &ExecServerCombineInventoryItems },
		{ TEXT("/Script/FortniteGame.FortPlayerController"), TEXT("ServerSpotActor"), &ExecServerSpotActor },
		{ TEXT("/Script/FortniteGame.FortPlayerController"), TEXT("ServerSetAutoEquipBetterItems"), &ExecServerSetAutoEquipBetterItems },
		{ TEXT("/Script/FortniteGame.FortWeapon"), TEXT("ServerReleaseWeaponAbility"), &ExecServerReleaseWeaponAbility },
	};
}

void FortBlueprintNatives::Install()
{
	static bool bInstalled = false;
	if (bInstalled)
	{
		return;
	}
	bInstalled = true;
	int32 Bound = 0;
	for (const FNativeBinding& Binding : GBindings)
	{
		UClass* Class = FindObject<UClass>(nullptr, Binding.ClassPath);
		UFunction* Function = Class ? Class->FindFunctionByName(Binding.FunctionName, EIncludeSuperFlag::ExcludeSuper) : nullptr;
		if (Function)
		{
			Function->SetNativeFunc(Binding.Exec);
			++Bound;
		}
		else
		{
			UE_LOG(LogFortBpNatives, Warning, TEXT("BpNatives: %s::%s not found, not rebound"), Binding.ClassPath, Binding.FunctionName);
		}
	}
	UE_LOG(LogFortBpNatives, Display, TEXT("BpNatives: %d of %d Blueprint natives rebound to server implementations"), Bound, UE_ARRAY_COUNT(GBindings));
}
