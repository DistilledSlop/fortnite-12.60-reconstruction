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
#include "WeaponHudKeyActionVisibility.h"
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
#include "Abilities/Tasks/AbilityTask.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AFortProjectileBase.h"
#include "ABuildingGameplayActor.h"
#include "UFortAbilityTask_SpawnProjectileAndWait.h"
#include "UFortProjectileMovementComponent.h"
#include "FFortGameplayEffectContainerSpec.h"
#include "FProjectileEventData.h"
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
#include "FortWeaponReload.h"
#include "AFortGameStateAthena.h"
#include "UFortPlaylistAthena.h"
#include "UOpenableInterface.h"
#include "UFortInventoryOwnerInterface.h"
#include "UFortWeaponItemDefinition.h"
#include "FFortGameplayEffectContainer.h"
#include "Abilities/GameplayAbility.h"
#include "EngineUtils.h"
#include "UFortGameplayAbility.h"
#include "ABuildingGameplayActorConsumable.h"
#include "ABuildingGameplayActorSentry.h"
#include "ABuildingGameplayActorSpawnMachine.h"
#include "AFortAthenaVehicleSpawner.h"
#include "AFortPlayerControllerAthena.h"
#include "UAthenaBattleBusItemDefinition.h"
#include "UFortVehicleItemDefinition.h"
#include "UFortHeldObjectComponent.h"
#include "UFortAsyncAction_GameStateReady.h"
#include "UFortAsyncAction_AuthorityMatchReady.h"
#include "UFortAsyncAction_CurrentPlaylistReady.h"
#include "UFortAsyncAction_TeleportAndWaitForLevelStreaming.h"
#include "UFortAbilityTask_PlayMontageWaitTarget.h"
#include "UFortAbilityTask_WaitTargetSelection.h"
#include "FFortGameplayAbilityMontageInfo.h"
#include "EFortAbilityTargetDataPolicy.h"
#include "EFortGameplayAbilityMontageSectionToPlay.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Animation/AnimMontage.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

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
		int64 Removed = 0;
		for (UFortWorldItem* Item : Items)
		{
			if (Amount >= 0 && Removed >= Amount)
			{
				break;
			}
			const int32 Have = Item->ItemEntry.Count;
			const int32 Take = Amount < 0 ? Have : FMath::Min(Have, Amount - static_cast<int32>(Removed));
			if (Take > 0)
			{
				Removed += FortAthenaInventory::RemoveItem(Controller, Item->NativeGetItemGuid(), Take);
			}
		}
		return static_cast<int32>(FMath::Min<int64>(Removed, MAX_int32));
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
		AFortInventory* Inventory = PlayerController ? PlayerController->WorldInventory : nullptr;
		UFortWorldItem* WorldItem = Cast<UFortWorldItem>(Item);
		if (Inventory && WorldItem && Inventory->NativeFindItemByGuid(WorldItem->NativeGetItemGuid()) == WorldItem)
		{
			Items.Add(WorldItem);
		}
		*static_cast<int32*>(RESULT_PARAM) = RemoveFromItems(PlayerController, Items, AmountToRemove);
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecK2_RemoveItemFromPlayerByGuid)
	{
		P_GET_OBJECT(AFortPlayerController, PlayerController);
		P_GET_STRUCT(FGuid, ItemGuid);
		P_GET_PROPERTY(FIntProperty, AmountToRemove);
		P_GET_UBOOL(bForceRemoval);
		P_FINISH;
		P_NATIVE_BEGIN;
		int32 Removed = 0;
		if (PlayerController && ItemGuid.IsValid() && AmountToRemove != 0)
		{
			Removed = FortAthenaInventory::RemoveItem(PlayerController, ItemGuid, AmountToRemove);
		}
		*static_cast<int32*>(RESULT_PARAM) = Removed;
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
		int64 Total = 0;
		for (const UFortWorldItem* Item : Items)
		{
			Total += Item->ItemEntry.Count;
		}
		*static_cast<int32*>(RESULT_PARAM) = static_cast<int32>(FMath::Min<int64>(Total, MAX_int32));
		P_NATIVE_END;
	}

	AFortPickup* SpawnPickupCommon(UObject* WorldContextObject, UFortWorldItemDefinition* ItemDefinition, int32 NumberToSpawn,
		const FVector& Position, const FVector& Direction, bool bToss, EFortPickupSourceTypeFlag SourceType,
		EFortPickupSpawnSource Source, AFortPlayerController* Owner, bool bOnlyRelevantToOwner,
		TSubclassOf<AFortPickup> PickupClass = nullptr)
	{
		UWorld* World = WorldOf(WorldContextObject);
		if (!World || !ItemDefinition || NumberToSpawn <= 0)
		{
			return nullptr;
		}
		// Use the same fallback distance as a tossed pickup without a pawn.
		constexpr float TossDistance = 180.0f;
		const FVector RestLocation = bToss ? Position + Direction.GetSafeNormal() * TossDistance : Position;
		AFortPickup* Pickup = FortAthenaLoot::SpawnResolvedPickup(World, RestLocation, ItemDefinition, NumberToSpawn, 1, 0,
			bToss ? &Position : nullptr, Owner, bOnlyRelevantToOwner, PickupClass);
		if (Pickup)
		{
			Pickup->PickupSourceTypeFlags = SourceType;
			Pickup->PickupSpawnSource = Source;
		}
		return Pickup;
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
		*static_cast<AFortPickup**>(RESULT_PARAM) = SpawnPickupCommon(WorldContextObject, ItemDefinition, NumberToSpawn,
			Position, Direction, bToss, SourceType, Source, OptionalOwnerPC, bPickupOnlyRelevantToOwner);
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
		*static_cast<AFortPickup**>(RESULT_PARAM) = SpawnPickupCommon(WorldContextObject, ItemDefinition, NumberToSpawn,
			Position, Direction, bToss, SourceType, Source, OptionalOwnerPC, bPickupOnlyRelevantToOwner, PickupClass);
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

	// ---- projectiles -------------------------------------------------------------------------------------------------------
	// stand-in: the original projectile spawn/hit/explode code (AFortProjectileBase native hit handling, effect-container application on
	// hit and explode) is not recovered. These functions spawn the projectile actor and give it its initial velocity so the Blueprint
	// flow (fuse timers, Blueprint explosion graphs) can run; hit/bounce/stop events that the retail native movement code raises are
	// not generated, so throwables that depend on them will not explode on impact yet.

	// Impact: the retail projectile forwards its movement component's stop event to the Blueprint OnStop event, which is where rockets,
	// impact grenades and traps start their explosion. The movement component raises the stop event on the server when the projectile
	// hits something it cannot bounce off (or slows below the bounce threshold), so bind it here. Both signatures carry one FHitResult.
	// stand-in: the retail native OnStop also records the stopping hit for client replication and arms the post-stop lifespan; bounce
	// and melee-hit events are not forwarded.
	void BindProjectileImpact(AFortProjectileBase* Projectile)
	{
		UProjectileMovementComponent* Movement = Projectile ? Projectile->ProjectileMovementComponent : nullptr;
		if (!Movement || !Projectile->HasAuthority())
		{
			return;
		}
		FScriptDelegate StopDelegate;
		StopDelegate.BindUFunction(Projectile, TEXT("OnStop"));
		Movement->OnProjectileStop.AddUnique(StopDelegate);
	}

	AFortProjectileBase* SpawnProjectileActor(TSubclassOf<AFortProjectileBase> ProjectileClass, AActor* RequestedBy, const FVector& Location,
		const FRotator& Rotation, const FRotator& Direction, float InitialSpeed, float GravityScale, AActor* HomingTarget, float ChargePercent, bool bFinish)
	{
		UWorld* World = RequestedBy ? RequestedBy->GetWorld() : nullptr;
		if (!World || !ProjectileClass)
		{
			return nullptr;
		}
		const FTransform Transform(Rotation, Location);
		AFortProjectileBase* Projectile = World->SpawnActorDeferred<AFortProjectileBase>(ProjectileClass, Transform, RequestedBy, Cast<APawn>(RequestedBy),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (!Projectile)
		{
			return nullptr;
		}
		Projectile->ChargePercent = ChargePercent;
		if (GravityScale > 0.0f)
		{
			Projectile->GravityScale = GravityScale;
		}
		Projectile->OriginalTarget = HomingTarget;
		if (UProjectileMovementComponent* Movement = Projectile->ProjectileMovementComponent)
		{
			if (InitialSpeed > 0.0f)
			{
				Movement->Velocity = Direction.Vector() * InitialSpeed;
			}
			if (GravityScale > 0.0f)
			{
				Movement->ProjectileGravityScale = GravityScale;
			}
			if (HomingTarget && HomingTarget->GetRootComponent())
			{
				Movement->HomingTargetComponent = HomingTarget->GetRootComponent();
			}
		}
		if (bFinish)
		{
			Projectile->FinishSpawning(Transform);
			BindProjectileImpact(Projectile);
		}
		return Projectile;
	}

	DEFINE_FUNCTION(ExecSpawnProjectile)
	{
		P_GET_OBJECT(UClass, ProjectileClass);
		P_GET_OBJECT(AActor, RequestedBy);
		P_GET_STRUCT_REF(FVector, SpawnLocation);
		P_GET_STRUCT(FRotator, SpawnRotation);
		P_GET_STRUCT_REF(FRotator, SpawnDirection);
		P_GET_STRUCT(FFortGameplayEffectContainerSpec, EffectContainerSpecToApplyOnHit);
		P_GET_STRUCT(FFortGameplayEffectContainerSpec, EffectContainerSpecToApplyOnExplode);
		P_GET_UBOOL(bUseDefaultPhysics);
		P_GET_OBJECT(AActor, HomingTarget);
		P_GET_PROPERTY(FFloatProperty, InitialSpeed);
		P_GET_PROPERTY(FFloatProperty, GravityScale);
		P_GET_PROPERTY(FFloatProperty, ChargePercent);
		P_FINISH;
		P_NATIVE_BEGIN;
		*static_cast<AFortProjectileBase**>(RESULT_PARAM) = SpawnProjectileActor(ProjectileClass, RequestedBy, SpawnLocation, SpawnRotation,
			SpawnDirection, InitialSpeed, GravityScale, HomingTarget, ChargePercent, true);
		P_NATIVE_END;
	}

	TMap<TWeakObjectPtr<UFortAbilityTask_SpawnProjectileAndWait>, TSubclassOf<AFortProjectileBase>>& TaskProjectileClasses()
	{
		static TMap<TWeakObjectPtr<UFortAbilityTask_SpawnProjectileAndWait>, TSubclassOf<AFortProjectileBase>> Map;
		return Map;
	}

	DEFINE_FUNCTION(ExecSpawnProjectileAndWait)
	{
		P_GET_OBJECT(UGameplayAbility, OwningAbility);
		P_GET_OBJECT(UClass, Class);
		P_GET_OBJECT(AActor, RequestedBy);
		P_GET_STRUCT_REF(FVector, SpawnLocation);
		P_GET_STRUCT_REF(FRotator, SpawnRotation);
		P_GET_STRUCT_REF(FRotator, SpawnDirection);
		P_GET_PROPERTY(FFloatProperty, InitialSpeed);
		P_GET_STRUCT(FFortGameplayEffectContainerSpec, EffectContainerSpecToApplyOnHit);
		P_GET_STRUCT(FFortGameplayEffectContainerSpec, EffectContainerSpecToApplyOnExplode);
		P_GET_PROPERTY(FFloatProperty, GravityScale);
		P_GET_OBJECT(AActor, HomingTarget);
		P_GET_UBOOL(bAllowProjectileSpawnWhenDead);
		P_GET_UBOOL(bAllowProjectileSpawnWhenDBNO);
		P_FINISH;
		P_NATIVE_BEGIN;
		UFortAbilityTask_SpawnProjectileAndWait* Task = OwningAbility ? UAbilityTask::NewAbilityTask<UFortAbilityTask_SpawnProjectileAndWait>(OwningAbility) : nullptr;
		if (Task)
		{
			Task->RequestedBy = RequestedBy;
			Task->SpawnLocation = SpawnLocation;
			Task->SpawnRotation = SpawnRotation;
			Task->SpawnDirection = SpawnDirection;
			Task->InitialSpeed = InitialSpeed;
			Task->EffectContainerSpecToApplyOnHit = EffectContainerSpecToApplyOnHit;
			Task->EffectContainerSpecToApplyOnExplode = EffectContainerSpecToApplyOnExplode;
			Task->GravityScale = GravityScale;
			Task->HomingTarget = HomingTarget;
			Task->bAllowSpawnWhenDead = bAllowProjectileSpawnWhenDead;
			Task->bAllowSpawnWhenDBNO = bAllowProjectileSpawnWhenDBNO;
			TaskProjectileClasses().Add(Task, Class);
		}
		*static_cast<UFortAbilityTask_SpawnProjectileAndWait**>(RESULT_PARAM) = Task;
		P_NATIVE_END;
	}

	// Latent-node pair of the task: Begin spawns the actor (deferred, so the Blueprint can set exposed values), Finish completes it.
	DEFINE_FUNCTION(ExecBeginSpawningActor)
	{
		P_GET_OBJECT(UGameplayAbility, OwningAbility);
		P_GET_OBJECT(UClass, Class);
		P_GET_OBJECT_REF(AFortProjectileBase, SpawnedActor);
		P_FINISH;
		P_NATIVE_BEGIN;
		UFortAbilityTask_SpawnProjectileAndWait* Task = Cast<UFortAbilityTask_SpawnProjectileAndWait>(Context);
		bool bSpawned = false;
		if (Task && Task->RequestedBy)
		{
			SpawnedActor = SpawnProjectileActor(Class, Task->RequestedBy, Task->SpawnLocation, Task->SpawnRotation, Task->SpawnDirection,
				Task->InitialSpeed, Task->GravityScale, Task->HomingTarget, 0.0f, false);
			Task->SpawnedProj = SpawnedActor;
			bSpawned = SpawnedActor != nullptr;
		}
		*static_cast<bool*>(RESULT_PARAM) = bSpawned;
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecFinishSpawningActor)
	{
		P_GET_OBJECT(UGameplayAbility, OwningAbility);
		P_GET_OBJECT(AFortProjectileBase, SpawnedActor);
		P_FINISH;
		P_NATIVE_BEGIN;
		UFortAbilityTask_SpawnProjectileAndWait* Task = Cast<UFortAbilityTask_SpawnProjectileAndWait>(Context);
		if (Task && SpawnedActor)
		{
			SpawnedActor->FinishSpawning(FTransform(Task->SpawnRotation, Task->SpawnLocation));
			BindProjectileImpact(SpawnedActor);
			FProjectileEventData EventData{};
			EventData.SpawnedProjectile = SpawnedActor;
			Task->Created.Broadcast(EventData);
		}
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecSpawnBuildingGameplayActor)
	{
		P_GET_OBJECT(UClass, BGAClass);
		P_GET_STRUCT_REF(FTransform, Transform);
		P_GET_OBJECT(AActor, Instigator);
		P_FINISH;
		P_NATIVE_BEGIN;
		ABuildingGameplayActor* Spawned = nullptr;
		UWorld* World = Instigator ? Instigator->GetWorld() : nullptr;
		if (World && BGAClass)
		{
			Spawned = World->SpawnActorDeferred<ABuildingGameplayActor>(BGAClass, Transform, Instigator, Cast<APawn>(Instigator),
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			if (Spawned)
			{
				Spawned->FinishSpawning(Transform);
			}
		}
		*static_cast<ABuildingGameplayActor**>(RESULT_PARAM) = Spawned;
		P_NATIVE_END;
	}

	// ---- squads, ammo rules, resurrection, open/close, hit flags, toss pickups ---------------------------------------------------

	TArray<AFortPlayerStateAthena*> SquadMembersOf(const AFortPlayerStateAthena* Of, bool bIncludeSelf)
	{
		TArray<AFortPlayerStateAthena*> Members;
		const UWorld* World = Of ? Of->GetWorld() : nullptr;
		const AGameStateBase* State = World ? World->GetGameState() : nullptr;
		if (!State)
		{
			return Members;
		}
		for (APlayerState* Candidate : State->PlayerArray)
		{
			AFortPlayerStateAthena* Member = Cast<AFortPlayerStateAthena>(Candidate);
			if (!Member || !IsValid(Member))
			{
				continue;
			}
			if (Member == Of)
			{
				if (bIncludeSelf)
				{
					Members.Add(Member);
				}
			}
			else if (Member->TeamIndex == Of->TeamIndex)
			{
				// stand-in: a squad is every player state of the same team; the retail native also filters on SquadId, which the
				// server assigns equal to the team's slot.
				Members.Add(Member);
			}
		}
		return Members;
	}

	DEFINE_FUNCTION(ExecGetSquadMemberPlayerStates)
	{
		P_GET_OBJECT(AFortPlayerStateAthena, InstigatingPlayerState);
		P_GET_UBOOL(bIncludeSelf);
		P_FINISH;
		P_NATIVE_BEGIN;
		*static_cast<TArray<AFortPlayerStateAthena*>*>(RESULT_PARAM) = SquadMembersOf(InstigatingPlayerState, bIncludeSelf);
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecIsInSquad)
	{
		P_FINISH;
		P_NATIVE_BEGIN;
		const AFortPlayerStateAthena* State = Cast<AFortPlayerStateAthena>(Context);
		*static_cast<bool*>(RESULT_PARAM) = State && SquadMembersOf(State, false).Num() > 0;
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecIsResurrectionEnabled)
	{
		P_GET_OBJECT(AFortPlayerPawn, PlayerPawn);
		P_FINISH;
		P_NATIVE_BEGIN;
		const AFortGameStateAthena* State = Cast<AFortGameStateAthena>(Context);
		bool bEnabled = State && State->ResurrectionEnabledRow.GetValueAtLevel(0.0f) > 0.0f;
		if (bEnabled && PlayerPawn)
		{
			const AFortPlayerStateAthena* Player = Cast<AFortPlayerStateAthena>(PlayerPawn->GetPlayerState());
			bEnabled = !(Player && Player->bResurrectionChipDisabled);
		}
		*static_cast<bool*>(RESULT_PARAM) = bEnabled;
		P_NATIVE_END;
	}

	// Same decision the weapon's ammo lookup (0x14269ed80) makes: a weapon that does not use ammo, or whose inventory owner has the
	// infinite-ammo rule (mutator / controller flag) for this weapon definition, never runs dry.
	bool WeaponOwnerHasInfiniteAmmo(AFortWeapon* Weapon)
	{
		IFortInventoryOwnerInterface* Owner = Weapon ? Weapon->NativeGetInventoryOwnerInterface14269c8f0() : nullptr;
		return Owner && Weapon->WeaponData && Owner->NativeHasInfiniteAmmo(Weapon->WeaponData);
	}

	DEFINE_FUNCTION(ExecHasInfiniteAmmo)
	{
		P_FINISH;
		P_NATIVE_BEGIN;
		AFortWeapon* Weapon = Cast<AFortWeapon>(Context);
		*static_cast<bool*>(RESULT_PARAM) = Weapon && (!Weapon->NativeUsesAmmo820() || WeaponOwnerHasInfiniteAmmo(Weapon));
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecInventoryOwnerHasInfiniteAmmo)
	{
		P_FINISH;
		P_NATIVE_BEGIN;
		*static_cast<bool*>(RESULT_PARAM) = WeaponOwnerHasInfiniteAmmo(Cast<AFortWeapon>(Context));
		P_NATIVE_END;
	}

	// Authority-only step a reload ability graph calls at its load point.
	DEFINE_FUNCTION(ExecTryReloadAmmoFromAbility)
	{
		P_FINISH;
		P_NATIVE_BEGIN;
		*static_cast<bool*>(RESULT_PARAM) = FortWeaponReload::LoadMagazine(Cast<AFortWeapon>(Context));
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecGetWeaponDataGameplayTags)
	{
		P_GET_STRUCT_REF(FGameplayTagContainer, GameplayTags);
		P_FINISH;
		P_NATIVE_BEGIN;
		const AFortWeapon* Weapon = Cast<AFortWeapon>(Context);
		GameplayTags = (Weapon && Weapon->WeaponData) ? Weapon->WeaponData->GameplayTags : FGameplayTagContainer();
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecIsCreativeOrPlaygroundModePlaylist)
	{
		P_GET_OBJECT(UObject, WorldContextObject);
		P_FINISH;
		P_NATIVE_BEGIN;
		UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
		const AFortGameStateAthena* State = World ? World->GetGameState<AFortGameStateAthena>() : nullptr;
		const UFortPlaylistAthena* Playlist = State ? State->GetCurrentPlaylistAthena() : nullptr;
		bool bCreative = false;
		if (Playlist)
		{
			const FString Name = Playlist->GetName();
			bCreative = Playlist->bEnableCreativeMode || Name.Contains(TEXT("Creative")) || Name.Contains(TEXT("Playground"));
		}
		*static_cast<bool*>(RESULT_PARAM) = bCreative;
		P_NATIVE_END;
	}

	// Doors and other openable actors implement IOpenableInterface (BlueprintNativeEvent Open/Close/IsOpen); these natives drive it and
	// report whether the actor ended up in the requested state.
	DEFINE_FUNCTION(ExecOpenActor)
	{
		P_GET_OBJECT(AActor, OpenableInterfaceActor);
		P_GET_OBJECT(AController, OptionalControllerInstigator);
		P_FINISH;
		P_NATIVE_BEGIN;
		bool bOpen = false;
		if (OpenableInterfaceActor && OpenableInterfaceActor->GetClass()->ImplementsInterface(UOpenableInterface::StaticClass()))
		{
			if (!IOpenableInterface::Execute_IsOpen(OpenableInterfaceActor))
			{
				IOpenableInterface::Execute_Open(OpenableInterfaceActor, OptionalControllerInstigator);
			}
			bOpen = IOpenableInterface::Execute_IsOpen(OpenableInterfaceActor);
		}
		*static_cast<bool*>(RESULT_PARAM) = bOpen;
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecCloseActor)
	{
		P_GET_OBJECT(AActor, OpenableInterfaceActor);
		P_GET_OBJECT(AController, OptionalControllerInstigator);
		P_FINISH;
		P_NATIVE_BEGIN;
		bool bClosed = false;
		if (OpenableInterfaceActor && OpenableInterfaceActor->GetClass()->ImplementsInterface(UOpenableInterface::StaticClass()))
		{
			if (IOpenableInterface::Execute_IsOpen(OpenableInterfaceActor))
			{
				IOpenableInterface::Execute_Close(OpenableInterfaceActor, OptionalControllerInstigator);
			}
			bClosed = !IOpenableInterface::Execute_IsOpen(OpenableInterfaceActor);
		}
		*static_cast<bool*>(RESULT_PARAM) = bClosed;
		P_NATIVE_END;
	}

	// Hit flags are derived the way the server damage path (FortHarvestFlow OnRangedTargetData) decides them: a hit on the head bone is
	// the critical zone, and a hit whose target has no health left is fatal.
	DEFINE_FUNCTION(ExecIsCriticalHit)
	{
		P_GET_STRUCT(FGameplayEffectContextHandle, EffectContext);
		P_FINISH;
		P_NATIVE_BEGIN;
		const FHitResult* Hit = EffectContext.IsValid() ? EffectContext.GetHitResult() : nullptr;
		*static_cast<bool*>(RESULT_PARAM) = Hit && Hit->BoneName.ToString().Contains(TEXT("head"), ESearchCase::IgnoreCase);
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecIsFatalHit)
	{
		P_GET_STRUCT(FGameplayEffectContextHandle, EffectContext);
		P_FINISH;
		P_NATIVE_BEGIN;
		const FHitResult* Hit = EffectContext.IsValid() ? EffectContext.GetHitResult() : nullptr;
		const AFortPawn* Victim = Hit ? Cast<AFortPawn>(Hit->GetActor()) : nullptr;
		*static_cast<bool*>(RESULT_PARAM) = Victim && Victim->GetHealth() <= 0.0f;
		P_NATIVE_END;
	}

	// The thrower makes a noise at the release point so perception systems (bots, hearing) can react to the throw.
	DEFINE_FUNCTION(ExecNotifyProjectileThrown)
	{
		P_GET_OBJECT(UClass, ProjClass);
		P_GET_OBJECT(AActor, RequestedBy);
		P_GET_STRUCT_REF(FVector, SpawnLocation);
		P_GET_STRUCT_REF(FRotator, SpawnRotation);
		P_GET_PROPERTY(FFloatProperty, Speed);
		P_FINISH;
		P_NATIVE_BEGIN;
		if (APawn* Thrower = Cast<APawn>(RequestedBy))
		{
			Thrower->MakeNoise(1.0f, Thrower, SpawnLocation, 0.0f, FName(TEXT("ProjectileThrown")));
		}
		P_NATIVE_END;
	}

	// Builds the runtime spec for an ability's effect container: one outgoing spec per target / owner effect class at the given level,
	// with the container's targeting, cue and magnitude settings carried over.
	DEFINE_FUNCTION(ExecMakeGameplayEffectContainerSpecFromAbility)
	{
		P_GET_STRUCT_REF(FFortGameplayEffectContainer, EffectContainer);
		P_GET_OBJECT(UGameplayAbility, Ability);
		P_GET_PROPERTY(FIntProperty, GameplayEffectLevel);
		P_FINISH;
		P_NATIVE_BEGIN;
		FFortGameplayEffectContainerSpec& Spec = *static_cast<FFortGameplayEffectContainerSpec*>(RESULT_PARAM);
		Spec = FFortGameplayEffectContainerSpec();
		Spec.TargetSelection = EffectContainer.TargetSelection;
		Spec.ActivationCues = EffectContainer.ActivationCues;
		Spec.ImpactCues = EffectContainer.ImpactCues;
		Spec.bOverrideChargeMagnitude = EffectContainer.bOverrideChargeMagnitude;
		Spec.ChargeMagnitudeOverrideValue = EffectContainer.ChargeMagnitudeOverrideValue;
		if (Ability)
		{
			const float Level = static_cast<float>(GameplayEffectLevel);
			for (const TSubclassOf<UGameplayEffect>& EffectClass : EffectContainer.TargetGameplayEffectClasses)
			{
				if (EffectClass)
				{
					Spec.TargetGameplayEffectSpecs.Add(Ability->MakeOutgoingGameplayEffectSpec(EffectClass, Level));
				}
			}
			for (const TSubclassOf<UGameplayEffect>& EffectClass : EffectContainer.OwnerGameplayEffectClasses)
			{
				if (EffectClass)
				{
					Spec.OwnerGameplayEffectSpecs.Add(Ability->MakeOutgoingGameplayEffectSpec(EffectClass, Level));
				}
			}
		}
		P_NATIVE_END;
	}

	// Starts the replicated toss of a pickup from its current position to Rest; the server moves the actor when the flight ends.
	// Same replicated fields the loot path uses (FortAthenaLoot::SpawnResolvedPickup).
	void BeginPickupToss(AFortPickup* Pickup, const FVector& Rest, AFortPawn* ItemOwner, bool bToss, bool bCombineWhenDone)
	{
		UWorld* World = Pickup ? Pickup->GetWorld() : nullptr;
		if (!World)
		{
			return;
		}
		const FVector From = Pickup->GetActorLocation();
		Pickup->bCombinePickupsWhenTossCompletes = bCombineWhenDone;
		if (!bToss)
		{
			Pickup->SetActorLocation(Rest);
			Pickup->PickupLocationData.TossState = EFortPickupTossState::AtRest;
			Pickup->ForceNetUpdate();
			return;
		}
		constexpr float TossFlyTime = 0.75f;
		FVector Dir = (Rest - From).GetSafeNormal();
		if (Dir.IsNearlyZero())
		{
			Dir = FVector::UpVector;
		}
		Pickup->PickupLocationData.ItemOwner = ItemOwner;
		Pickup->PickupLocationData.PickupTarget = nullptr;
		Pickup->PickupLocationData.CombineTarget = nullptr;
		Pickup->PickupLocationData.LootInitialPosition = From;
		Pickup->PickupLocationData.LootFinalPosition = Rest;
		Pickup->PickupLocationData.FinalTossRestLocation = Rest;
		Pickup->PickupLocationData.FlyTime = TossFlyTime;
		Pickup->PickupLocationData.StartDirection = Dir;
		Pickup->PickupLocationData.TossState = EFortPickupTossState::InProgress;
		Pickup->PickupLocationData.bPlayPickupSound = false;
		Pickup->PickupLocationData.PickupGuid = FGuid::NewGuid();
		Pickup->ForceNetUpdate();
		FTimerHandle RestTimer;
		TWeakObjectPtr<AFortPickup> WeakPickup(Pickup);
		World->GetTimerManager().SetTimer(RestTimer, [WeakPickup, Rest, bCombineWhenDone]()
		{
			AFortPickup* Target = WeakPickup.Get();
			if (!Target || !IsValid(Target) || Target->bPickedUp)
			{
				return;
			}
			Target->SetActorLocation(Rest);
			Target->PickupLocationData.TossState = EFortPickupTossState::AtRest;
			if (bCombineWhenDone)
			{
				// Merge into a resting pickup of the same item (same definition and level) within reach, up to the stack limit.
				FFortItemEntry& Mine = Target->PrimaryPickupItemEntry;
				const int32 MaxStack = Mine.ItemDefinition ? FMath::Max(1, static_cast<int32>(Mine.ItemDefinition->MaxStackSize.GetValue())) : 1;
				for (TActorIterator<AFortPickup> It(Target->GetWorld()); It; ++It)
				{
					AFortPickup* Other = *It;
					if (Other == Target || !IsValid(Other) || Other->bPickedUp || Other->PickupLocationData.TossState != EFortPickupTossState::AtRest)
					{
						continue;
					}
					FFortItemEntry& Theirs = Other->PrimaryPickupItemEntry;
					if (Theirs.ItemDefinition == Mine.ItemDefinition && Theirs.Level == Mine.Level
						&& FVector::DistSquared(Other->GetActorLocation(), Rest) < FMath::Square(150.0f)
						&& Theirs.Count + Mine.Count <= MaxStack)
					{
						Theirs.Count += Mine.Count;
						Other->ForceNetUpdate();
						Target->Destroy();
						return;
					}
				}
			}
			Target->ForceNetUpdate();
		}, TossFlyTime, false);
	}

	DEFINE_FUNCTION(ExecTossPickup)
	{
		P_GET_STRUCT(FVector, FinalLocation);
		P_GET_OBJECT(AFortPawn, ItemOwner);
		P_GET_PROPERTY(FIntProperty, OverrideMaxStackCount);
		P_GET_UBOOL(bToss);
		P_GET_UBOOL(bShouldCombinePickupsWhenTossCompletes);
		P_GET_ENUM(EFortPickupSourceTypeFlag, InPickupSourceTypeFlags);
		P_GET_ENUM(EFortPickupSpawnSource, InPickupSpawnSource);
		P_FINISH;
		P_NATIVE_BEGIN;
		if (AFortPickup* Pickup = Cast<AFortPickup>(Context))
		{
			Pickup->PickupSourceTypeFlags = InPickupSourceTypeFlags;
			Pickup->PickupSpawnSource = InPickupSpawnSource;
			// stand-in: OverrideMaxStackCount only limits how a later combine may grow the stack in the retail native; the combine step
			// above uses the item definition's own stack limit.
			BeginPickupToss(Pickup, FinalLocation, ItemOwner, bToss, bShouldCombinePickupsWhenTossCompletes);
		}
		P_NATIVE_END;
	}

	// Spawns the ammo pickup that belongs to a weapon definition and tosses it away from Location (used by throwables and drops).
	DEFINE_FUNCTION(ExecCreateTossAmmoPickup)
	{
		P_GET_OBJECT(UObject, WorldContextObject);
		P_GET_OBJECT(UFortWeaponItemDefinition, WeaponItemDefinition);
		P_GET_STRUCT_REF(FGameplayTagContainer, SourceTags);
		P_GET_STRUCT_REF(FVector, Location);
		P_GET_ENUM(EFortPickupSourceTypeFlag, SourceTypeFlag);
		P_GET_ENUM(EFortPickupSpawnSource, SpawnSource);
		P_FINISH;
		P_NATIVE_BEGIN;
		UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
		UFortWorldItemDefinition* Ammo = WeaponItemDefinition ? WeaponItemDefinition->GetAmmoWorldItemDefinitionNative() : nullptr;
		if (World && Ammo)
		{
			const int32 Count = FMath::Max(1, Ammo->DropCount);
			const float Angle = FMath::FRandRange(0.0f, 2.0f * PI);
			const FVector Rest = Location + FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.0f) * FMath::FRandRange(80.0f, 160.0f);
			const FVector From = Location + FVector(0.0f, 0.0f, 40.0f);
			if (AFortPickup* Pickup = FortAthenaLoot::SpawnResolvedPickup(World, Rest, Ammo, Count, 0, 0, &From))
			{
				Pickup->PickupSourceTypeFlags = SourceTypeFlag;
				Pickup->PickupSpawnSource = SpawnSource;
			}
		}
		P_NATIVE_END;
	}

	// ---- consumables, building helpers, async-ready actions, montage / targeting ability tasks ------------------------------------

	void AfterSeconds(UWorld* World, float Delay, TFunction<void()> Function)
	{
		if (World)
		{
			FTimerHandle Handle;
			World->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda(MoveTemp(Function)), FMath::Max(Delay, 0.01f), false);
		}
	}

	// Calls Fire once Ready() holds, checking a few times a second. The Blueprint binds its delegates after the factory returns, so the
	// first check is deferred past the current frame.
	void FireWhenReady(UWorld* World, TFunction<bool()> Ready, TFunction<void()> Fire)
	{
		if (!World)
		{
			return;
		}
		TSharedRef<FTimerHandle> Handle = MakeShared<FTimerHandle>();
		TWeakObjectPtr<UWorld> WeakWorld(World);
		World->GetTimerManager().SetTimer(*Handle, FTimerDelegate::CreateLambda([WeakWorld, Handle, Ready, Fire]()
		{
			UWorld* Current = WeakWorld.Get();
			if (!Current)
			{
				return;
			}
			if (Ready())
			{
				Current->GetTimerManager().ClearTimer(*Handle);
				Fire();
			}
		}), 0.25f, true, 0.05f);
	}

	DEFINE_FUNCTION(ExecGetOnConsumeMontage)
	{
		P_FINISH;
		P_NATIVE_BEGIN;
		const ABuildingGameplayActorConsumable* Consumable = Cast<ABuildingGameplayActorConsumable>(Context);
		*static_cast<UAnimMontage**>(RESULT_PARAM) = Consumable ? Consumable->OnConsumeMontage : nullptr;
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecGetVehicleClass)
	{
		P_FINISH;
		P_NATIVE_BEGIN;
		const AFortAthenaVehicleSpawner* Spawner = Cast<AFortAthenaVehicleSpawner>(Context);
		UFortVehicleItemDefinition* Definition = Spawner ? Spawner->FortVehicleItemDef.LoadSynchronous() : nullptr;
		UClass* VehicleClass = Definition ? Definition->VehicleActorClass.LoadSynchronous() : nullptr;
		*static_cast<TSubclassOf<UObject>*>(RESULT_PARAM) = VehicleClass;
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecGetCooldownLength)
	{
		P_FINISH;
		P_NATIVE_BEGIN;
		const ABuildingGameplayActorSpawnMachine* Machine = Cast<ABuildingGameplayActorSpawnMachine>(Context);
		*static_cast<float*>(RESULT_PARAM) = Machine ? Machine->CooldownLengthRow.GetValueAtLevel(0.0f) : 0.0f;
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecSetSentryState)
	{
		P_GET_ENUM(EBuildingGameplayActorSentry_State, InSentryState);
		P_FINISH;
		P_NATIVE_BEGIN;
		if (ABuildingGameplayActorSentry* Sentry = Cast<ABuildingGameplayActorSentry>(Context))
		{
			if (Sentry->CurrentSentryState != InSentryState)
			{
				Sentry->CurrentSentryState = InSentryState;
				Sentry->ForceNetUpdate();
				Sentry->OnRep_CurrentSentryState();
			}
		}
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecSetPawnBeingTracked)
	{
		P_GET_OBJECT(AFortPlayerPawn, InPawnBeingTracked);
		P_FINISH;
		P_NATIVE_BEGIN;
		if (ABuildingGameplayActorSentry* Sentry = Cast<ABuildingGameplayActorSentry>(Context))
		{
			Sentry->PawnBeingTracked = InPawnBeingTracked;
			if (InPawnBeingTracked)
			{
				Sentry->LastPawnBeingTrackedLocation = InPawnBeingTracked->GetActorLocation();
			}
		}
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecForceChangeOwnerInstigator)
	{
		P_GET_OBJECT(AFortPlayerController, NewInstigator);
		P_FINISH;
		P_NATIVE_BEGIN;
		if (AActor* Self = Cast<AActor>(Context))
		{
			Self->SetOwner(NewInstigator);
			Self->SetInstigator(NewInstigator ? NewInstigator->GetPawn() : nullptr);
		}
		P_NATIVE_END;
	}

	// A building that Blueprint code kills (hiding props, trap spawners): goes through the same damage path as a lethal hit so loot,
	// resources, cues and death events all run.
	DEFINE_FUNCTION(ExecBlueprintDie)
	{
		P_GET_STRUCT(FGameplayTag, DeathReason);
		P_GET_OBJECT(AController, Killer);
		P_GET_OBJECT(AActor, DamageCauser);
		P_FINISH;
		P_NATIVE_BEGIN;
		if (ABuildingActor* Building = Cast<ABuildingActor>(Context))
		{
			if (!Building->bDestroyed)
			{
				if (ABuildingSMActor* Piece = Cast<ABuildingSMActor>(Building))
				{
					const float Lethal = FMath::Max(FortHarvest::GetBuildingMaxHealth(Piece), FortHarvest::GetBuildingHealth(Piece)) + 1.0f;
					FortHarvest::ApplyBuildingDamage(Piece, Lethal, Killer, DamageCauser, FHitResult(), false);
				}
				else
				{
					Building->bDestroyed = 1;
					Building->Destroy();
				}
			}
		}
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecGetLootTierGroupOverride)
	{
		P_FINISH;
		P_NATIVE_BEGIN;
		const ABuildingContainer* Container = Cast<ABuildingContainer>(Context);
		*static_cast<FName*>(RESULT_PARAM) = Container ? Container->SearchLootTierGroup : FName();
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecGetBattleBusForPlayer)
	{
		P_GET_OBJECT(AFortPlayerControllerAthena, PC);
		P_FINISH;
		P_NATIVE_BEGIN;
		*static_cast<UAthenaBattleBusItemDefinition**>(RESULT_PARAM) = PC ? PC->CosmeticLoadoutPC.BattleBus : nullptr;
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecApplyGlobalEnvironmentGameplayEffectToActor)
	{
		P_GET_OBJECT(AActor, EffectTargetActor);
		P_GET_OBJECT(UClass, GameplayEffect);
		P_GET_PROPERTY(FIntProperty, GameplayEffectLevel);
		P_GET_STRUCT(FGameplayTagContainer, AdditionalContextTags);
		P_FINISH;
		P_NATIVE_BEGIN;
		FActiveGameplayEffectHandle Handle;
		UAbilitySystemComponent* ASC = nullptr;
		if (EffectTargetActor)
		{
			const IAbilitySystemInterface* Interface = Cast<IAbilitySystemInterface>(EffectTargetActor);
			ASC = Interface ? Interface->GetAbilitySystemComponent() : EffectTargetActor->FindComponentByClass<UAbilitySystemComponent>();
		}
		if (ASC && GameplayEffect && GameplayEffect->IsChildOf(UGameplayEffect::StaticClass()))
		{
			const FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(GameplayEffect, static_cast<float>(GameplayEffectLevel), ASC->MakeEffectContext());
			if (Spec.IsValid())
			{
				Spec.Data->CapturedSourceTags.GetSpecTags().AppendTags(AdditionalContextTags);
				Handle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			}
		}
		*static_cast<FActiveGameplayEffectHandle*>(RESULT_PARAM) = Handle;
		P_NATIVE_END;
	}

	AActor* FindHeldObjectActor(const AFortPlayerPawn* Pawn)
	{
		if (Pawn)
		{
			TArray<AActor*> Attached;
			Pawn->GetAttachedActors(Attached);
			for (AActor* Actor : Attached)
			{
				if (Actor && Actor->FindComponentByClass<UFortHeldObjectComponent>())
				{
					return Actor;
				}
			}
		}
		return nullptr;
	}

	DEFINE_FUNCTION(ExecGetHeldObjectAttachedToPlayer)
	{
		P_GET_OBJECT(AFortPlayerPawn, FortPawn);
		P_FINISH;
		P_NATIVE_BEGIN;
		*static_cast<AActor**>(RESULT_PARAM) = FindHeldObjectActor(FortPawn);
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecGetHeldObjectComponentAttachedToPlayer)
	{
		P_GET_OBJECT(AFortPlayerPawn, FortPawn);
		P_FINISH;
		P_NATIVE_BEGIN;
		const AActor* Held = FindHeldObjectActor(FortPawn);
		*static_cast<UFortHeldObjectComponent**>(RESULT_PARAM) = Held ? Held->FindComponentByClass<UFortHeldObjectComponent>() : nullptr;
		P_NATIVE_END;
	}

	// A held object can be picked up by a pawn that is not already carrying one, while nobody carries this object.
	DEFINE_FUNCTION(ExecHeldObjectCanInteract)
	{
		P_GET_OBJECT(AFortPlayerPawn, RequestingPawn);
		P_FINISH;
		P_NATIVE_BEGIN;
		const UFortHeldObjectComponent* Component = Cast<UFortHeldObjectComponent>(Context);
		const AActor* Owner = Component ? Component->GetOwner() : nullptr;
		*static_cast<bool*>(RESULT_PARAM) = Owner && RequestingPawn && !Owner->GetAttachParentActor() && !FindHeldObjectActor(RequestingPawn);
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecProjectileSetVelocity)
	{
		P_GET_STRUCT_REF(FVector, InReplicatedVelocity);
		P_FINISH;
		P_NATIVE_BEGIN;
		if (UProjectileMovementComponent* Movement = Cast<UProjectileMovementComponent>(Context))
		{
			Movement->Velocity = InReplicatedVelocity;
			Movement->UpdateComponentVelocity();
		}
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecFortShippingLog)
	{
		P_GET_OBJECT(UObject, WorldContextObject);
		P_GET_PROPERTY(FStrProperty, InString);
		P_GET_UBOOL(bLogAsWarning);
		P_FINISH;
		P_NATIVE_BEGIN;
		if (bLogAsWarning)
		{
			UE_LOG(LogFortBpNatives, Warning, TEXT("BP: %s"), *InString);
		}
		else
		{
			UE_LOG(LogFortBpNatives, Log, TEXT("BP: %s"), *InString);
		}
		P_NATIVE_END;
	}

	// The async "ready" nodes complete as soon as the server-side condition holds; each broadcasts once and releases itself.
	DEFINE_FUNCTION(ExecGameStateReadyAsync)
	{
		P_GET_OBJECT(UObject, WorldContextObject);
		P_FINISH;
		P_NATIVE_BEGIN;
		UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
		UFortAsyncAction_GameStateReady* Action = World ? NewObject<UFortAsyncAction_GameStateReady>(GetTransientPackage()) : nullptr;
		if (Action)
		{
			Action->RegisterWithGameInstance(WorldContextObject);
			TWeakObjectPtr<UFortAsyncAction_GameStateReady> Weak(Action);
			TWeakObjectPtr<UWorld> WeakWorld(World);
			FireWhenReady(World, [WeakWorld]() { return WeakWorld.IsValid() && WeakWorld->GetGameState() != nullptr; },
				[Weak, WeakWorld]()
				{
					if (UFortAsyncAction_GameStateReady* Target = Weak.Get())
					{
						Target->OnReady.Broadcast(WeakWorld.IsValid() ? WeakWorld->GetGameState() : nullptr);
						Target->SetReadyToDestroy();
					}
				});
		}
		*static_cast<UFortAsyncAction_GameStateReady**>(RESULT_PARAM) = Action;
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecMatchReadyAsync)
	{
		P_GET_OBJECT(UObject, WorldContextObject);
		P_FINISH;
		P_NATIVE_BEGIN;
		UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
		UFortAsyncAction_AuthorityMatchReady* Action = World ? NewObject<UFortAsyncAction_AuthorityMatchReady>(GetTransientPackage()) : nullptr;
		if (Action)
		{
			Action->RegisterWithGameInstance(WorldContextObject);
			TWeakObjectPtr<UFortAsyncAction_AuthorityMatchReady> Weak(Action);
			TWeakObjectPtr<UWorld> WeakWorld(World);
			FireWhenReady(World, [WeakWorld]()
				{
					const AFortGameStateAthena* State = WeakWorld.IsValid() ? WeakWorld->GetGameState<AFortGameStateAthena>() : nullptr;
					return State && State->bPlaylistDataIsLoaded && WeakWorld->GetAuthGameMode() != nullptr;
				},
				[Weak]()
				{
					if (UFortAsyncAction_AuthorityMatchReady* Target = Weak.Get())
					{
						Target->OnReady.Broadcast();
						Target->SetReadyToDestroy();
					}
				});
		}
		*static_cast<UFortAsyncAction_AuthorityMatchReady**>(RESULT_PARAM) = Action;
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecPlaylistDataReadyAsync)
	{
		P_GET_OBJECT(UObject, WorldContextObject);
		P_FINISH;
		P_NATIVE_BEGIN;
		UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
		UFortAsyncAction_CurrentPlaylistReady* Action = World ? NewObject<UFortAsyncAction_CurrentPlaylistReady>(GetTransientPackage()) : nullptr;
		if (Action)
		{
			Action->RegisterWithGameInstance(WorldContextObject);
			TWeakObjectPtr<UFortAsyncAction_CurrentPlaylistReady> Weak(Action);
			TWeakObjectPtr<UWorld> WeakWorld(World);
			FireWhenReady(World, [WeakWorld]()
				{
					const AFortGameStateAthena* State = WeakWorld.IsValid() ? WeakWorld->GetGameState<AFortGameStateAthena>() : nullptr;
					return State && State->bPlaylistDataIsLoaded && State->GetCurrentPlaylistAthena() != nullptr;
				},
				[Weak, WeakWorld]()
				{
					AFortGameStateAthena* State = WeakWorld.IsValid() ? WeakWorld->GetGameState<AFortGameStateAthena>() : nullptr;
					if (UFortAsyncAction_CurrentPlaylistReady* Target = Weak.Get())
					{
						Target->OnReady.Broadcast(State, State ? State->GetCurrentPlaylistAthena() : nullptr, FGameplayTagContainer());
						Target->SetReadyToDestroy();
					}
				});
		}
		*static_cast<UFortAsyncAction_CurrentPlaylistReady**>(RESULT_PARAM) = Action;
		P_NATIVE_END;
	}

	// Waits until streaming levels around the destination have finished before reporting the teleport complete.
	DEFINE_FUNCTION(ExecCreateTeleportAndWaitForLevelStreaming)
	{
		P_GET_OBJECT(AFortPlayerPawn, InPlayerPawn);
		P_GET_STRUCT_REF(FVector, InTeleportLocation);
		P_GET_STRUCT_REF(FRotator, InTeleportRotation);
		P_FINISH;
		P_NATIVE_BEGIN;
		UWorld* World = InPlayerPawn ? InPlayerPawn->GetWorld() : nullptr;
		UFortAsyncAction_TeleportAndWaitForLevelStreaming* Action = World ? NewObject<UFortAsyncAction_TeleportAndWaitForLevelStreaming>(GetTransientPackage()) : nullptr;
		if (Action)
		{
			Action->PlayerPawn = InPlayerPawn;
			Action->TeleportLocation = InTeleportLocation;
			Action->TeleportRotation = InTeleportRotation;
			Action->RegisterWithGameInstance(InPlayerPawn);
			TWeakObjectPtr<UFortAsyncAction_TeleportAndWaitForLevelStreaming> Weak(Action);
			TWeakObjectPtr<UWorld> WeakWorld(World);
			AfterSeconds(World, 0.02f, [Weak]()
			{
				if (UFortAsyncAction_TeleportAndWaitForLevelStreaming* Target = Weak.Get())
				{
					if (Target->PlayerPawn)
					{
						Target->PlayerPawn->TeleportTo(Target->TeleportLocation, Target->TeleportRotation);
					}
				}
			});
			FireWhenReady(World, [WeakWorld]() { return WeakWorld.IsValid() && !WeakWorld->IsVisibilityRequestPending(); },
				[Weak]()
				{
					if (UFortAsyncAction_TeleportAndWaitForLevelStreaming* Target = Weak.Get())
					{
						Target->OnComplete.Broadcast();
						Target->SetReadyToDestroy();
					}
				});
		}
		*static_cast<UFortAsyncAction_TeleportAndWaitForLevelStreaming**>(RESULT_PARAM) = Action;
		P_NATIVE_END;
	}

	FGameplayAbilityTargetDataHandle SelfTargetData(const UAbilityTask* Task)
	{
		FGameplayAbilityTargetDataHandle Handle;
		if (const AActor* Avatar = (Task && Task->AbilitySystemComponent) ? Task->AbilitySystemComponent->GetAvatarActor() : nullptr)
		{
			FGameplayAbilityTargetData_ActorArray* Data = new FGameplayAbilityTargetData_ActorArray();
			Data->TargetActorArray.Add(const_cast<AActor*>(Avatar));
			Handle.Add(Data);
		}
		return Handle;
	}

	// Plays the montage on the ability's avatar and reports Triggered near its end (where the retail montage notifies consumption)
	// and Completed when it finishes. Both stop if the ability ended earlier.
	UFortAbilityTask_PlayMontageWaitTarget* StartMontageTask(UGameplayAbility* OwningAbility, FName TaskInstanceName, UAnimMontage* Montage, float Rate,
		FName Section, EFortAbilityTargetDataPolicy Policy)
	{
		UFortAbilityTask_PlayMontageWaitTarget* Task = OwningAbility ? UAbilityTask::NewAbilityTask<UFortAbilityTask_PlayMontageWaitTarget>(OwningAbility, TaskInstanceName) : nullptr;
		UWorld* World = OwningAbility ? OwningAbility->GetWorld() : nullptr;
		if (!Task || !World)
		{
			return Task;
		}
		TWeakObjectPtr<UFortAbilityTask_PlayMontageWaitTarget> Weak(Task);
		AfterSeconds(World, 0.02f, [Weak, OwningAbilityWeak = TWeakObjectPtr<UGameplayAbility>(OwningAbility), Montage, Rate, Section, World]()
		{
			UFortAbilityTask_PlayMontageWaitTarget* Target = Weak.Get();
			UGameplayAbility* Ability = OwningAbilityWeak.Get();
			if (!Target || !Ability || !Target->IsActive() || !Target->AbilitySystemComponent)
			{
				return;
			}
			// Cooked consumable montage info can leave the play rate at zero.
			const float PlayRate = FMath::IsNearlyZero(Rate) ? 1.0f : Rate;
			float Duration = Montage ? Target->AbilitySystemComponent->PlayMontage(Ability, Ability->GetCurrentActivationInfo(), Montage, PlayRate, Section) : 0.0f;
			if (Duration <= 0.0f)
			{
				Duration = Montage ? Montage->GetPlayLength() / FMath::Abs(PlayRate) : 0.0f;
			}
			Duration = FMath::Max(Duration, 0.05f);
			AfterSeconds(World, Duration * 0.9f, [Weak]()
			{
				if (UFortAbilityTask_PlayMontageWaitTarget* Active = Weak.Get())
				{
					if (Active->IsActive())
					{
						Active->Triggered.Broadcast(SelfTargetData(Active), FGameplayTag());
					}
				}
			});
			AfterSeconds(World, Duration, [Weak]()
			{
				if (UFortAbilityTask_PlayMontageWaitTarget* Active = Weak.Get())
				{
					if (Active->IsActive())
					{
						Active->Completed.Broadcast(FGameplayAbilityTargetDataHandle(), FGameplayTag());
						Active->EndTask();
					}
				}
			});
		});
		return Task;
	}

	FName PickMontageSection(UAnimMontage* Montage, EFortGameplayAbilityMontageSectionToPlay Choice, FName OverrideSection, bool bRandom)
	{
		if (!OverrideSection.IsNone())
		{
			return OverrideSection;
		}
		if ((bRandom || Choice != EFortGameplayAbilityMontageSectionToPlay::FirstSection) && Montage && Montage->CompositeSections.Num() > 0)
		{
			return Montage->CompositeSections[FMath::RandRange(0, Montage->CompositeSections.Num() - 1)].SectionName;
		}
		return NAME_None;
	}

	DEFINE_FUNCTION(ExecPlayMontageWaitTarget)
	{
		P_GET_OBJECT(UGameplayAbility, OwningAbility);
		P_GET_PROPERTY(FNameProperty, TaskInstanceName);
		P_GET_OBJECT(UAnimMontage, MontageToPlay);
		P_GET_PROPERTY(FFloatProperty, AnimPlayRate);
		P_GET_ENUM(EFortGameplayAbilityMontageSectionToPlay, SectionToPlay);
		P_GET_PROPERTY(FNameProperty, OverrideSection);
		P_GET_PROPERTY(FFloatProperty, AnimRootMotionTranslationScale);
		P_GET_ENUM(EFortAbilityTargetDataPolicy, TargetDataPolicy);
		P_FINISH;
		P_NATIVE_BEGIN;
		UFortAbilityTask_PlayMontageWaitTarget* Task = StartMontageTask(OwningAbility, TaskInstanceName, MontageToPlay, AnimPlayRate,
			PickMontageSection(MontageToPlay, SectionToPlay, OverrideSection, false), TargetDataPolicy);
		*static_cast<UFortAbilityTask_PlayMontageWaitTarget**>(RESULT_PARAM) = Task;
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecPlayMontageInfoAndWaitForTrigger)
	{
		P_GET_OBJECT(UGameplayAbility, OwningAbility);
		P_GET_PROPERTY(FNameProperty, TaskInstanceName);
		P_GET_STRUCT_REF(FFortGameplayAbilityMontageInfo, MontageInfo);
		P_GET_ENUM(EFortAbilityTargetDataPolicy, TargetDataPolicy);
		P_FINISH;
		P_NATIVE_BEGIN;
		UFortAbilityTask_PlayMontageWaitTarget* Task = StartMontageTask(OwningAbility, TaskInstanceName, MontageInfo.MontageToPlay, MontageInfo.AnimPlayRate,
			PickMontageSection(MontageInfo.MontageToPlay, MontageInfo.MontageSectionToPlay, MontageInfo.OverrideSection, MontageInfo.bPlayRandomSection),
			TargetDataPolicy);
		if (Task)
		{
			Task->MontageInfo = MontageInfo;
		}
		*static_cast<UFortAbilityTask_PlayMontageWaitTarget**>(RESULT_PARAM) = Task;
		P_NATIVE_END;
	}

	// Instant targeting: the avatar itself is the selected target, reported as soon as the Blueprint has bound the delegates.
	DEFINE_FUNCTION(ExecWaitTargetSelection)
	{
		P_GET_OBJECT(UGameplayAbility, OwningAbility);
		P_GET_PROPERTY(FNameProperty, TaskInstanceName);
		P_GET_STRUCT(FGameplayTag, InApplicationTag);
		P_GET_UBOOL(bInForceTargetingOnServer);
		P_FINISH;
		P_NATIVE_BEGIN;
		UFortAbilityTask_WaitTargetSelection* Task = OwningAbility ? UAbilityTask::NewAbilityTask<UFortAbilityTask_WaitTargetSelection>(OwningAbility, TaskInstanceName) : nullptr;
		if (Task)
		{
			Task->bForceTargetingOnServer = bInForceTargetingOnServer;
			Task->InstantTargetTag = InApplicationTag;
			TWeakObjectPtr<UFortAbilityTask_WaitTargetSelection> Weak(Task);
			AfterSeconds(OwningAbility->GetWorld(), 0.05f, [Weak, InApplicationTag]()
			{
				if (UFortAbilityTask_WaitTargetSelection* Active = Weak.Get())
				{
					if (Active->IsActive())
					{
						Active->Targeted.Broadcast(SelfTargetData(Active), InApplicationTag);
						Active->EndTask();
					}
				}
			});
		}
		*static_cast<UFortAbilityTask_WaitTargetSelection**>(RESULT_PARAM) = Task;
		P_NATIVE_END;
	}

	// ---- UFortGameplayAbility effect containers (base of every ability; a Blueprint calling one of these undeclared virtual-by-name
	// functions aborts the server: "Failed to find function ApplyGameplayEffectContainer in GAB_Melee_...") -----------------------

	FFortGameplayEffectContainerSpec BuildContainerSpec(const FFortGameplayEffectContainer& Container, UGameplayAbility* Ability, int32 Level)
	{
		FFortGameplayEffectContainerSpec Spec;
		Spec.TargetSelection = Container.TargetSelection;
		Spec.ActivationCues = Container.ActivationCues;
		Spec.ImpactCues = Container.ImpactCues;
		Spec.bOverrideChargeMagnitude = Container.bOverrideChargeMagnitude;
		Spec.ChargeMagnitudeOverrideValue = Container.ChargeMagnitudeOverrideValue;
		if (Ability)
		{
			for (const TSubclassOf<UGameplayEffect>& EffectClass : Container.TargetGameplayEffectClasses)
			{
				if (EffectClass)
				{
					Spec.TargetGameplayEffectSpecs.Add(Ability->MakeOutgoingGameplayEffectSpec(EffectClass, static_cast<float>(Level)));
				}
			}
			for (const TSubclassOf<UGameplayEffect>& EffectClass : Container.OwnerGameplayEffectClasses)
			{
				if (EffectClass)
				{
					Spec.OwnerGameplayEffectSpecs.Add(Ability->MakeOutgoingGameplayEffectSpec(EffectClass, static_cast<float>(Level)));
				}
			}
		}
		return Spec;
	}

	// Containers of an ability that apply under ApplicationTag: the fixed slots, the configured list and the runtime-added ones.
	void CollectContainers(const UFortGameplayAbility* Ability, const FGameplayTag& ApplicationTag, TArray<FFortGameplayEffectContainer>& Out)
	{
		if (!Ability)
		{
			return;
		}
		for (const FFortGameplayEffectContainer& Container : Ability->EffectContainers)
		{
			if (Container.ApplicationTag == ApplicationTag)
			{
				Out.Add(Container);
			}
		}
		for (const FFortGameplayEffectContainer& Container : Ability->GameplayEffectContainers)
		{
			if (Container.ApplicationTag == ApplicationTag)
			{
				Out.Add(Container);
			}
		}
		if (const FFortGameplayEffectContainer* Dynamic = Ability->DynamicGameplayEffectContainers.Find(ApplicationTag))
		{
			Out.Add(*Dynamic);
		}
	}

	DEFINE_FUNCTION(ExecBP_GetGameplayEffectContainers)
	{
		P_GET_STRUCT(FGameplayTag, ApplicationTag);
		P_GET_TARRAY_REF(FFortGameplayEffectContainer, OutContainers);
		P_FINISH;
		P_NATIVE_BEGIN;
		OutContainers.Reset();
		CollectContainers(Cast<UFortGameplayAbility>(Context), ApplicationTag, OutContainers);
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecBP_GetGameplayEffectContainer)
	{
		P_GET_STRUCT(FGameplayTag, ApplicationTag);
		P_FINISH;
		P_NATIVE_BEGIN;
		TArray<FFortGameplayEffectContainer> Found;
		CollectContainers(Cast<UFortGameplayAbility>(Context), ApplicationTag, Found);
		*static_cast<FFortGameplayEffectContainer*>(RESULT_PARAM) = Found.Num() > 0 ? Found[0] : FFortGameplayEffectContainer();
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecMakeGameplayEffectContainerSpecs)
	{
		P_GET_STRUCT(FGameplayTag, ApplicationTag);
		P_GET_PROPERTY(FIntProperty, GameplayEffectLevel);
		P_GET_TARRAY_REF(FFortGameplayEffectContainerSpec, OutSpecContainers);
		P_FINISH;
		P_NATIVE_BEGIN;
		OutSpecContainers.Reset();
		UFortGameplayAbility* Ability = Cast<UFortGameplayAbility>(Context);
		TArray<FFortGameplayEffectContainer> Found;
		CollectContainers(Ability, ApplicationTag, Found);
		for (const FFortGameplayEffectContainer& Container : Found)
		{
			OutSpecContainers.Add(BuildContainerSpec(Container, Ability, GameplayEffectLevel));
		}
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecMakeGameplayEffectContainerSpec)
	{
		P_GET_STRUCT(FGameplayTag, ApplicationTag);
		P_GET_PROPERTY(FIntProperty, GameplayEffectLevel);
		P_FINISH;
		P_NATIVE_BEGIN;
		UFortGameplayAbility* Ability = Cast<UFortGameplayAbility>(Context);
		TArray<FFortGameplayEffectContainer> Found;
		CollectContainers(Ability, ApplicationTag, Found);
		*static_cast<FFortGameplayEffectContainerSpec*>(RESULT_PARAM) = Found.Num() > 0 ? BuildContainerSpec(Found[0], Ability, GameplayEffectLevel) : FFortGameplayEffectContainerSpec();
		P_NATIVE_END;
	}

	TArray<FActiveGameplayEffectHandle> ApplyContainers(UFortGameplayAbility* Ability, const FGameplayAbilityTargetDataHandle& TargetData,
		const FGameplayTag& ApplicationTag, int32 Level)
	{
		TArray<FActiveGameplayEffectHandle> Applied;
		TArray<FFortGameplayEffectContainer> Found;
		CollectContainers(Ability, ApplicationTag, Found);
		for (const FFortGameplayEffectContainer& Container : Found)
		{
			const FFortGameplayEffectContainerSpec Spec = BuildContainerSpec(Container, Ability, Level);
			UAbilitySystemComponent* OwnerASC = Ability->GetAbilitySystemComponentFromActorInfo();
			for (const FGameplayEffectSpecHandle& SpecHandle : Spec.TargetGameplayEffectSpecs)
			{
				if (!SpecHandle.IsValid() || !OwnerASC)
				{
					continue;
				}
				for (const TSharedPtr<FGameplayAbilityTargetData>& Data : TargetData.Data)
				{
					if (!Data.IsValid())
					{
						continue;
					}
					for (const TWeakObjectPtr<AActor>& Actor : Data->GetActors())
					{
						const IAbilitySystemInterface* Interface = Cast<IAbilitySystemInterface>(Actor.Get());
						UAbilitySystemComponent* TargetASC = Interface ? Interface->GetAbilitySystemComponent() : nullptr;
						if (TargetASC)
						{
							FGameplayEffectSpec TargetSpec(*SpecHandle.Data.Get());
							if (TargetSpec.GetContext().IsValid())
							{
								// Keep this hit's context isolated from the next target and effect.
								FGameplayEffectContextHandle TargetContext = TargetSpec.GetContext().Duplicate();
								if (const FHitResult* Hit = Data->GetHitResult())
								{
									// A hit inherited from the outgoing spec can belong to a different actor.
									TargetContext.AddHitResult(*Hit, true);
								}
								Data->AddTargetDataToContext(TargetContext, false);
								TargetSpec.SetContext(TargetContext);
							}
							Applied.Add(OwnerASC->ApplyGameplayEffectSpecToTarget(TargetSpec, TargetASC));
						}
					}
				}
			}
			for (const FGameplayEffectSpecHandle& SpecHandle : Spec.OwnerGameplayEffectSpecs)
			{
				if (SpecHandle.IsValid() && OwnerASC)
				{
					Applied.Add(OwnerASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get()));
				}
			}
		}
		return Applied;
	}

	DEFINE_FUNCTION(ExecApplyGameplayEffectContainer)
	{
		P_GET_STRUCT(FGameplayAbilityTargetDataHandle, TargetData);
		P_GET_STRUCT(FGameplayTag, ApplicationTag);
		P_GET_PROPERTY(FIntProperty, GameplayEffectLevel);
		P_FINISH;
		P_NATIVE_BEGIN;
		*static_cast<TArray<FActiveGameplayEffectHandle>*>(RESULT_PARAM) = ApplyContainers(Cast<UFortGameplayAbility>(Context), TargetData, ApplicationTag, GameplayEffectLevel);
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecApplyGameplayEffectContainerWithCSVStats)
	{
		P_GET_STRUCT(FGameplayAbilityTargetDataHandle, TargetData);
		P_GET_STRUCT(FGameplayTag, ApplicationTag);
		P_GET_PROPERTY(FIntProperty, GameplayEffectLevel);
		P_GET_PROPERTY(FStrProperty, StatName);
		P_FINISH;
		P_NATIVE_BEGIN;
		*static_cast<TArray<FActiveGameplayEffectHandle>*>(RESULT_PARAM) = ApplyContainers(Cast<UFortGameplayAbility>(Context), TargetData, ApplicationTag, GameplayEffectLevel);
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecDoesTargetHaveTags)
	{
		P_GET_OBJECT(AActor, Target);
		P_GET_STRUCT_REF(FGameplayTagContainer, Tags);
		P_FINISH;
		P_NATIVE_BEGIN;
		const IAbilitySystemInterface* Interface = Cast<IAbilitySystemInterface>(Target);
		UAbilitySystemComponent* ASC = Interface ? Interface->GetAbilitySystemComponent() : nullptr;
		*static_cast<bool*>(RESULT_PARAM) = ASC && ASC->HasAllMatchingGameplayTags(Tags);
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecGetCurrentAbilitySourceLevel)
	{
		P_GET_PROPERTY_REF(FFloatProperty, OutSourceLevel);
		P_FINISH;
		P_NATIVE_BEGIN;
		const UGameplayAbility* Ability = Cast<UGameplayAbility>(Context);
		OutSourceLevel = Ability ? static_cast<float>(FMath::Max(Ability->GetAbilityLevel(), 1)) : 1.0f;
		P_NATIVE_END;
	}

	// Where an ability fires from: the avatar's eye point and the controller's aim.
	DEFINE_FUNCTION(ExecGetFireLocation)
	{
		P_FINISH;
		P_NATIVE_BEGIN;
		const UGameplayAbility* Ability = Cast<UGameplayAbility>(Context);
		const AActor* Avatar = Ability ? Ability->GetAvatarActorFromActorInfo() : nullptr;
		const APawn* Pawn = Cast<APawn>(Avatar);
		*static_cast<FVector*>(RESULT_PARAM) = Pawn ? Pawn->GetPawnViewLocation() : (Avatar ? Avatar->GetActorLocation() : FVector::ZeroVector);
		P_NATIVE_END;
	}

	DEFINE_FUNCTION(ExecGetFireRotation)
	{
		P_GET_UBOOL(bApplyLeading);
		P_GET_UBOOL(bApplyInaccuracy);
		P_GET_UBOOL(bApplyClampedMaxYawAngleToFire);
		P_FINISH;
		P_NATIVE_BEGIN;
		const UGameplayAbility* Ability = Cast<UGameplayAbility>(Context);
		const AActor* Avatar = Ability ? Ability->GetAvatarActorFromActorInfo() : nullptr;
		const APawn* Pawn = Cast<APawn>(Avatar);
		*static_cast<FRotator*>(RESULT_PARAM) = Pawn ? Pawn->GetViewRotation() : (Avatar ? Avatar->GetActorRotation() : FRotator::ZeroRotator);
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

	// ServerReturnToMainMenu: the "Return to Lobby" click (G9 plan section 4). binary: original body 0x1423ac090 logs
	// "FortPC::ServerReturnToMainMenu()" and invokes virtual +0xa58, which resolves to the engine RPC
	// ClientReturnToMainMenuWithTextReason; the client owns the frontend transition, so only that reliable RPC is sent (empty reason).
	// stand-in: the original also starts a timeout and touches session ownership, which is not reproduced; other clients and the match
	// are left running.
	DEFINE_FUNCTION(ExecServerReturnToMainMenu)
	{
		P_FINISH;
		P_NATIVE_BEGIN;
		if (AFortPlayerController* Controller = Cast<AFortPlayerController>(Context))
		{
			UE_LOG(LogFortBpNatives, Display, TEXT("FortPC::ServerReturnToMainMenu() %s"), *Controller->GetName());
			Controller->ClientReturnToMainMenuWithTextReason(FText::GetEmpty());
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
		{ TEXT("/Script/FortniteGame.FortGameplayAbility"), TEXT("BP_GetGameplayEffectContainers"), &ExecBP_GetGameplayEffectContainers },
		{ TEXT("/Script/FortniteGame.FortGameplayAbility"), TEXT("BP_GetGameplayEffectContainer"), &ExecBP_GetGameplayEffectContainer },
		{ TEXT("/Script/FortniteGame.FortGameplayAbility"), TEXT("MakeGameplayEffectContainerSpecs"), &ExecMakeGameplayEffectContainerSpecs },
		{ TEXT("/Script/FortniteGame.FortGameplayAbility"), TEXT("MakeGameplayEffectContainerSpec"), &ExecMakeGameplayEffectContainerSpec },
		{ TEXT("/Script/FortniteGame.FortGameplayAbility"), TEXT("ApplyGameplayEffectContainer"), &ExecApplyGameplayEffectContainer },
		{ TEXT("/Script/FortniteGame.FortGameplayAbility"), TEXT("ApplyGameplayEffectContainerWithCSVStats"), &ExecApplyGameplayEffectContainerWithCSVStats },
		{ TEXT("/Script/FortniteGame.FortGameplayAbility"), TEXT("DoesTargetHaveTags"), &ExecDoesTargetHaveTags },
		{ TEXT("/Script/FortniteGame.FortGameplayAbility"), TEXT("GetCurrentAbilitySourceLevel"), &ExecGetCurrentAbilitySourceLevel },
		{ TEXT("/Script/FortniteGame.FortGameplayAbility"), TEXT("GetFireLocation"), &ExecGetFireLocation },
		{ TEXT("/Script/FortniteGame.FortGameplayAbility"), TEXT("GetFireRotation"), &ExecGetFireRotation },
		{ TEXT("/Script/FortniteGame.BuildingGameplayActorConsumable"), TEXT("GetOnConsumeMontage"), &ExecGetOnConsumeMontage },
		{ TEXT("/Script/FortniteGame.FortAthenaVehicleSpawner"), TEXT("GetVehicleClass"), &ExecGetVehicleClass },
		{ TEXT("/Script/FortniteGame.BuildingGameplayActorSpawnMachine"), TEXT("GetCooldownLength"), &ExecGetCooldownLength },
		{ TEXT("/Script/FortniteGame.BuildingGameplayActorSentry"), TEXT("SetSentryState"), &ExecSetSentryState },
		{ TEXT("/Script/FortniteGame.BuildingGameplayActorSentry"), TEXT("SetPawnBeingTracked"), &ExecSetPawnBeingTracked },
		{ TEXT("/Script/FortniteGame.BuildingGameplayActor"), TEXT("ForceChangeOwnerInstigator"), &ExecForceChangeOwnerInstigator },
		{ TEXT("/Script/FortniteGame.BuildingActor"), TEXT("BlueprintDie"), &ExecBlueprintDie },
		{ TEXT("/Script/FortniteGame.BuildingActor"), TEXT("GetLootTierGroupOverride"), &ExecGetLootTierGroupOverride },
		{ TEXT("/Script/FortniteGame.FortGameStateAthena"), TEXT("GetBattleBusForPlayer"), &ExecGetBattleBusForPlayer },
		{ TEXT("/Script/FortniteGame.FortKismetLibrary"), TEXT("ApplyGlobalEnvironmentGameplayEffectToActor"), &ExecApplyGlobalEnvironmentGameplayEffectToActor },
		{ TEXT("/Script/FortniteGame.FortKismetLibrary"), TEXT("GetHeldObjectAttachedToPlayer"), &ExecGetHeldObjectAttachedToPlayer },
		{ TEXT("/Script/FortniteGame.FortKismetLibrary"), TEXT("GetHeldObjectComponentAttachedToPlayer"), &ExecGetHeldObjectComponentAttachedToPlayer },
		{ TEXT("/Script/FortniteGame.FortKismetLibrary"), TEXT("FortShippingLog"), &ExecFortShippingLog },
		{ TEXT("/Script/FortniteGame.FortHeldObjectComponent"), TEXT("CanInteract"), &ExecHeldObjectCanInteract },
		{ TEXT("/Script/FortniteGame.FortProjectileMovementComponent"), TEXT("SetVelocity"), &ExecProjectileSetVelocity },
		{ TEXT("/Script/FortniteGame.FortAsyncAction_GameStateReady"), TEXT("GameStateReadyAsync"), &ExecGameStateReadyAsync },
		{ TEXT("/Script/FortniteGame.FortAsyncAction_AuthorityMatchReady"), TEXT("MatchReadyAsync"), &ExecMatchReadyAsync },
		{ TEXT("/Script/FortniteGame.FortAsyncAction_CurrentPlaylistReady"), TEXT("PlaylistDataReadyAsync"), &ExecPlaylistDataReadyAsync },
		{ TEXT("/Script/FortniteGame.FortAsyncAction_TeleportAndWaitForLevelStreaming"), TEXT("CreateTeleportAndWaitForLevelStreaming"), &ExecCreateTeleportAndWaitForLevelStreaming },
		{ TEXT("/Script/FortniteGame.FortAbilityTask_PlayMontageWaitTarget"), TEXT("PlayMontageWaitTarget"), &ExecPlayMontageWaitTarget },
		{ TEXT("/Script/FortniteGame.FortAbilityTask_PlayMontageWaitTarget"), TEXT("PlayMontageInfoAndWaitForTrigger"), &ExecPlayMontageInfoAndWaitForTrigger },
		{ TEXT("/Script/FortniteGame.FortAbilityTask_WaitTargetSelection"), TEXT("WaitTargetSelection"), &ExecWaitTargetSelection },
		{ TEXT("/Script/FortniteGame.FortKismetLibrary"), TEXT("IsCriticalHit"), &ExecIsCriticalHit },
		{ TEXT("/Script/FortniteGame.FortKismetLibrary"), TEXT("IsFatalHit"), &ExecIsFatalHit },
		{ TEXT("/Script/FortniteGame.FortKismetLibrary"), TEXT("OpenActor"), &ExecOpenActor },
		{ TEXT("/Script/FortniteGame.FortKismetLibrary"), TEXT("CloseActor"), &ExecCloseActor },
		{ TEXT("/Script/FortniteGame.FortKismetLibrary"), TEXT("NotifyProjectileThrown"), &ExecNotifyProjectileThrown },
		{ TEXT("/Script/FortniteGame.FortKismetLibrary"), TEXT("MakeGameplayEffectContainerSpecFromAbility"), &ExecMakeGameplayEffectContainerSpecFromAbility },
		{ TEXT("/Script/FortniteGame.FortKismetLibrary"), TEXT("IsCreativeOrPlaygroundModePlaylist"), &ExecIsCreativeOrPlaygroundModePlaylist },
		{ TEXT("/Script/FortniteGame.FortKismetLibrary"), TEXT("CreateTossAmmoPickupForWeaponItemDefinitionAtLocation"), &ExecCreateTossAmmoPickup },
		{ TEXT("/Script/FortniteGame.FortPickup"), TEXT("TossPickup"), &ExecTossPickup },
		{ TEXT("/Script/FortniteGame.FortWeapon"), TEXT("TryReloadAmmoFromAbility"), &ExecTryReloadAmmoFromAbility },
		{ TEXT("/Script/FortniteGame.FortWeapon"), TEXT("HasInfiniteAmmo"), &ExecHasInfiniteAmmo },
		{ TEXT("/Script/FortniteGame.FortWeapon"), TEXT("InventoryOwnerHasInfiniteAmmo"), &ExecInventoryOwnerHasInfiniteAmmo },
		{ TEXT("/Script/FortniteGame.FortWeapon"), TEXT("GetWeaponDataGameplayTags"), &ExecGetWeaponDataGameplayTags },
		{ TEXT("/Script/FortniteGame.FortPlayerStateAthena"), TEXT("IsInSquad"), &ExecIsInSquad },
		{ TEXT("/Script/FortniteGame.FortGameStateAthena"), TEXT("GetSquadMemberPlayerStates"), &ExecGetSquadMemberPlayerStates },
		{ TEXT("/Script/FortniteGame.FortGameStateAthena"), TEXT("IsResurrectionEnabled"), &ExecIsResurrectionEnabled },
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
		{ TEXT("/Script/FortniteGame.FortKismetLibrary"), TEXT("K2_RemoveItemFromPlayerByGuid"), &ExecK2_RemoveItemFromPlayerByGuid },
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
		{ TEXT("/Script/FortniteGame.FortKismetLibrary"), TEXT("SpawnProjectile"), &ExecSpawnProjectile },
		{ TEXT("/Script/FortniteGame.FortKismetLibrary"), TEXT("SpawnBuildingGameplayActor"), &ExecSpawnBuildingGameplayActor },
		{ TEXT("/Script/FortniteGame.FortAbilityTask_SpawnProjectileAndWait"), TEXT("SpawnProjectileAndWait"), &ExecSpawnProjectileAndWait },
		{ TEXT("/Script/FortniteGame.FortAbilityTask_SpawnProjectileAndWait"), TEXT("BeginSpawningActor"), &ExecBeginSpawningActor },
		{ TEXT("/Script/FortniteGame.FortAbilityTask_SpawnProjectileAndWait"), TEXT("FinishSpawningActor"), &ExecFinishSpawningActor },
		{ TEXT("/Script/FortniteGame.FortPlayerController"), TEXT("ServerReturnToMainMenu"), &ExecServerReturnToMainMenu },
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
