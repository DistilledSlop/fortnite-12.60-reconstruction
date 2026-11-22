#include "FortAthenaLootTransfer.h"

#include "AFortInventory.h"
#include "AFortPawn.h"
#include "AFortPickup.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AFortPickupAthena.h"
#include "AFortPlayerController.h"
#include "AFortPlayerControllerAthena.h"
#include "AFortPlayerPawn.h"
#include "AFortPlayerPawnAthena.h"
#include "AFortWeapon.h"
#include "Components/SceneComponent.h"
#include "CollisionQueryParams.h"
#include "EFortDelayedQuickBarAction.h"
#include "EFortItemType.h"
#include "EFortPickupSourceTypeFlag.h"
#include "EFortPickupTossState.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "EWorldItemDropBehavior.h"
#include "FDelayedQuickBarAction.h"
#include "FFortItemEntry.h"
#include "FQuickBarData.h"
#include "FortAthenaInventoryFlow.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/ScopeExit.h"
#include "TimerManager.h"
#include "UFortItemDefinition.h"
#include "UFortWeaponRangedItemDefinition.h"
#include "UFortWorldItem.h"
#include "UFortWorldItemDefinition.h"
#include "UObject/ScriptMacros.h"

namespace
{
	constexpr int32 PrimarySlotCount = 6;   // Slot 0 = pickaxe/harvesting tool, Slots 1..5 = weapons/consumables
	constexpr int32 SecondarySlotCount = 5; // Slots 0..3 = building pieces, Slot 4 = traps
	constexpr int32 CreativeSlotCount = 8;
	constexpr float MaxPickupRangeSq = 600.0f * 600.0f; // stand-in: range guard replacing bDoServerHandlePickupTrace

	TSet<AFortPickup*> PickupsBeingGranted;
	TSet<TWeakObjectPtr<AFortPickup>> BlockedAutoPickups;
	constexpr float AutoPickupRangeSq = 125.0f * 125.0f;

	struct FSlotLocation
	{
		EFortQuickBars Bar = EFortQuickBars::Max_None;
		int32 Slot = INDEX_NONE;

		bool IsValid() const { return Bar != EFortQuickBars::Max_None && Slot >= 0; }
	};

	// Per-controller server-side mirror of quickbar slot occupancy and monotonic ActionId counter
	// matching FDelayedQuickBarActionContainer::NextActionId (binary 0x1423679c0, offset +0x118).
	struct FPlayerQuickBarState
	{
		TArray<FGuid> PrimarySlots;
		TArray<FGuid> SecondarySlots;
		TArray<FGuid> CreativeSlots;
		uint32 NextActionId = 0;

		FPlayerQuickBarState()
		{
			PrimarySlots.SetNum(PrimarySlotCount);
			SecondarySlots.SetNum(SecondarySlotCount);
			CreativeSlots.SetNum(CreativeSlotCount);
		}

		TArray<FGuid>* GetBarArray(EFortQuickBars Bar)
		{
			switch (Bar)
			{
			case EFortQuickBars::Primary:   return &PrimarySlots;
			case EFortQuickBars::Secondary: return &SecondarySlots;
			case EFortQuickBars::Creative:  return &CreativeSlots;
			default:                        return nullptr;
			}
		}

		const TArray<FGuid>* GetBarArray(EFortQuickBars Bar) const
		{
			return const_cast<FPlayerQuickBarState*>(this)->GetBarArray(Bar);
		}
	};

	TMap<TWeakObjectPtr<AFortPlayerController>, FPlayerQuickBarState> GQuickBarStates;

	void PruneInvalidControllers()
	{
		for (auto It = GQuickBarStates.CreateIterator(); It; ++It)
		{
			AFortPlayerController* PC = It.Key().Get();
			if (!IsValid(PC))
			{
				It.RemoveCurrent();
			}
		}
	}

	// Returns item count helper and definition helper safely.
	UFortItemDefinition* GetEntryDef(const FFortItemEntry& Entry)
	{
		return Entry.ItemDefinition;
	}

	EFortItemType GetQuickBarItemType(const UFortItemDefinition* Definition)
	{
		const EFortItemType Type = Definition->NativeGetItemType();
		// Cooked ranged assets may inherit an untyped or base weapon value when ItemType is omitted.
		if ((Type == EFortItemType::WorldItem || Type == EFortItemType::Max_None || Type == EFortItemType::WeaponMelee) &&
			Definition->IsA<UFortWeaponRangedItemDefinition>())
		{
			return EFortItemType::WeaponRanged;
		}
		return Type;
	}

	bool IsQuickBarEligibleType(EFortItemType ItemType, const UFortWorldItemDefinition* WorldDef)
	{
		switch (ItemType)
		{
		case EFortItemType::WeaponHarvest:
		case EFortItemType::Weapon:
		case EFortItemType::WeaponMelee:
		case EFortItemType::WeaponRanged:
		case EFortItemType::WeaponCreativePhone:
		case EFortItemType::Consumable:
		case EFortItemType::Food:
		case EFortItemType::Gadget:
		case EFortItemType::AthenaGadget:
		case EFortItemType::Deco:
		case EFortItemType::ItemCache:
		case EFortItemType::BackpackPickup:
		case EFortItemType::BuildingPiece:
		case EFortItemType::Trap:
			return true;
		case EFortItemType::WorldItem:
			return WorldDef != nullptr && WorldDef->bSupportsQuickbarFocus;
		default:
			return false;
		}
	}

	// Determines target quickbar and allowed slot index range [MinSlot, MaxSlot] for an item definition.
	// Uses AFortPlayerControllerAthena::QuickBarDefinitions when populated, otherwise canonical Athena BR layout
	// derived from 0x1419d3cf0 / 0x1423bd5e0 / 0x142049610.
	bool ResolveQuickBarRange(const AFortPlayerController* Controller, const UFortItemDefinition* Def,
		EFortQuickBars& OutBar, int32& OutMinSlot, int32& OutMaxSlot, int32& OutMaxSlotsInBar, const FQuickBarData*& OutBarDefinition)
	{
		OutBar = EFortQuickBars::Max_None;
		OutMinSlot = INDEX_NONE;
		OutMaxSlot = INDEX_NONE;
		OutMaxSlotsInBar = 0;
		OutBarDefinition = nullptr;

		if (!Def)
		{
			return false;
		}

		const UFortWorldItemDefinition* WorldDef = Cast<UFortWorldItemDefinition>(Def);
		const EFortItemType ItemType = GetQuickBarItemType(Def);
		if (!IsQuickBarEligibleType(ItemType, WorldDef))
		{
			return false;
		}

		if (const AFortPlayerControllerAthena* AthenaPC = Cast<AFortPlayerControllerAthena>(Controller))
		{
			for (int32 BarIdx = 0; BarIdx < 3; ++BarIdx)
			{
				const FQuickBarData& BarDef = AthenaPC->QuickBarDefinitions[BarIdx];
				const int32 NumSlots = BarDef.QuickbarSlots.Num();
				if (NumSlots <= 0)
				{
					continue;
				}
				int32 FirstMatch = INDEX_NONE;
				int32 LastMatch = INDEX_NONE;
				for (int32 SlotIdx = 0; SlotIdx < NumSlots; ++SlotIdx)
				{
					if (BarDef.QuickbarSlots[SlotIdx].AcceptedItemTypes.Contains(ItemType))
					{
						if (FirstMatch == INDEX_NONE)
						{
							FirstMatch = SlotIdx;
						}
						LastMatch = SlotIdx;
					}
				}
				if (FirstMatch != INDEX_NONE)
				{
					OutBar = static_cast<EFortQuickBars>(BarIdx);
					OutMinSlot = FirstMatch;
					OutMaxSlot = LastMatch;
					OutMaxSlotsInBar = NumSlots;
					OutBarDefinition = &BarDef;
					return true;
				}
			}
		}

		// stand-in: Athena_PlayerController Blueprint QuickBarDefinitions defaults are not loaded into native CDO;
		// canonical BR 12.60 layout: Primary[0]=Pickaxe, Primary[1..5]=Weapons/Consumables, Secondary[0..3]=Building, Secondary[4]=Trap.
		switch (ItemType)
		{
		case EFortItemType::WeaponHarvest:
			OutBar = EFortQuickBars::Primary;
			OutMinSlot = 0;
			OutMaxSlot = 0;
			OutMaxSlotsInBar = PrimarySlotCount;
			return true;

		case EFortItemType::BuildingPiece:
			OutBar = EFortQuickBars::Secondary;
			OutMinSlot = 0;
			OutMaxSlot = 3;
			OutMaxSlotsInBar = SecondarySlotCount;
			return true;

		case EFortItemType::Trap:
			OutBar = EFortQuickBars::Secondary;
			OutMinSlot = 4;
			OutMaxSlot = 4;
			OutMaxSlotsInBar = SecondarySlotCount;
			return true;

		case EFortItemType::WeaponCreativePhone:
			OutBar = EFortQuickBars::Creative;
			OutMinSlot = 0;
			OutMaxSlot = 0;
			OutMaxSlotsInBar = CreativeSlotCount;
			return true;

		default:
			OutBar = EFortQuickBars::Primary;
			OutMinSlot = 1;
			OutMaxSlot = PrimarySlotCount - 1;
			OutMaxSlotsInBar = PrimarySlotCount;
			return true;
		}
	}

	FPlayerQuickBarState& GetOrCreateState(AFortPlayerController* Controller)
	{
		PruneInvalidControllers();
		FPlayerQuickBarState& State = GQuickBarStates.FindOrAdd(Controller);
		return State;
	}

	// Removes stale GUIDs no longer present in Controller->WorldInventory so server slot state never drifts.
	void PruneStaleSlotGuids(AFortPlayerController* Controller, FPlayerQuickBarState& State)
	{
		if (!Controller || !Controller->WorldInventory)
		{
			return;
		}
		auto PruneBar = [Controller](TArray<FGuid>& Slots)
		{
			for (FGuid& SlotGuid : Slots)
			{
				if (!SlotGuid.IsValid())
				{
					continue;
				}
				UFortWorldItem* Existing = Controller->WorldInventory->NativeFindItemByGuid(SlotGuid);
				if (!Existing || Existing->ItemEntry.Count <= 0)
				{
					SlotGuid.Invalidate();
				}
			}
		};
		PruneBar(State.PrimarySlots);
		PruneBar(State.SecondarySlots);
		PruneBar(State.CreativeSlots);
	}

	FSlotLocation FindItemSlot(const FPlayerQuickBarState& State, const FGuid& ItemGuid)
	{
		if (!ItemGuid.IsValid())
		{
			return {};
		}
		const EFortQuickBars Bars[] = { EFortQuickBars::Primary, EFortQuickBars::Secondary, EFortQuickBars::Creative };
		for (EFortQuickBars Bar : Bars)
		{
			const TArray<FGuid>* Slots = State.GetBarArray(Bar);
			if (!Slots)
			{
				continue;
			}
			for (int32 i = 0; i < Slots->Num(); ++i)
			{
				if ((*Slots)[i] == ItemGuid)
				{
					return { Bar, i };
				}
			}
		}
		return {};
	}

	int32 GetItemSlotSpan(UFortWorldItem* Item)
	{
		if (!Item)
		{
			return 1;
		}
		const int32 Span = static_cast<int32>(Item->GetNumberOfSlotsTaken());
		return FMath::Max(1, Span);
	}

	// Checks for Span consecutive empty slots that accept the item type.
	int32 FindFreeSlotRange(const TArray<FGuid>& Slots, int32 MinSlot, int32 MaxSlot, int32 Span,
		const FQuickBarData* BarDefinition, EFortItemType ItemType)
	{
		const int32 ClampedMin = FMath::Max(0, MinSlot);
		const int32 ClampedMax = FMath::Min(MaxSlot, Slots.Num() - 1);
		if (Span <= 0 || ClampedMax - ClampedMin + 1 < Span)
		{
			return INDEX_NONE;
		}

		for (int32 Start = ClampedMin; Start <= ClampedMax - Span + 1; ++Start)
		{
			bool bAllFree = true;
			for (int32 Offset = 0; Offset < Span; ++Offset)
			{
				const int32 Slot = Start + Offset;
				if (Slots[Slot].IsValid() ||
					(BarDefinition && !BarDefinition->QuickbarSlots[Slot].AcceptedItemTypes.Contains(ItemType)))
				{
					bAllFree = false;
					break;
				}
			}
			if (bAllFree)
			{
				return Start;
			}
		}
		return INDEX_NONE;
	}

	void OccupySlots(TArray<FGuid>& Slots, int32 HeadSlot, int32 Span, const FGuid& Guid)
	{
		for (int32 Offset = 0; Offset < Span; ++Offset)
		{
			const int32 Idx = HeadSlot + Offset;
			if (Slots.IsValidIndex(Idx))
			{
				Slots[Idx] = Guid;
			}
		}
	}

	void ClearGuidFromSlots(TArray<FGuid>& Slots, const FGuid& Guid)
	{
		for (FGuid& EntryGuid : Slots)
		{
			if (EntryGuid == Guid)
			{
				EntryGuid.Invalidate();
			}
		}
	}

	// Enqueues a replicated quickbar delta matching 0x1423684c0 / 0x1423679c0.
	// On the client, OnRep_DelayedQuickBarActions (0x142a48ee0 -> 0x1423a5190) executes the action
	// against ClientQuickBars and sends ServerAcknowledgeDelayedQuickBarAction.
	void EnqueueDelayedQuickBarAction(AFortPlayerController* Controller, FPlayerQuickBarState& State,
		EFortDelayedQuickBarAction ActionType, const FGuid& ItemGuid, UFortItemDefinition* Def,
		EFortQuickBars QuickBarType, int32 QuickBarSlot, bool bForceExecution)
	{
		if (!Controller || !Controller->HasAuthority() || !ItemGuid.IsValid() || !Def)
		{
			return;
		}

		FDelayedQuickBarAction Action;
		Action.ActionId = ++State.NextActionId;
		Action.Action = ActionType;
		Action.ItemGuid = ItemGuid;
		Action.ItemDefinitionAssetId = Def->GetPrimaryAssetId();
		Action.QuickBarType = QuickBarType;
		Action.QuickBarSlot = QuickBarSlot;
		Action.bForceExecution = bForceExecution;
		Action.ReplicationID = INDEX_NONE;
		Action.ReplicationKey = INDEX_NONE;

		const int32 NewIdx = Controller->DelayedQuickBarActions.Items.Add(Action);
		Controller->DelayedQuickBarActions.MarkItemDirty(Controller->DelayedQuickBarActions.Items[NewIdx]);
		Controller->ForceNetUpdate();

		UE_LOG(LogTemp, Display, TEXT("LootTransfer: QuickBar action=%d id=%u pc=%s def=%s guid=%s bar=%d slot=%d"),
			static_cast<int32>(ActionType),
			Action.ActionId,
			*Controller->GetName(),
			*Def->GetName(),
			*ItemGuid.ToString(),
			static_cast<int32>(QuickBarType),
			QuickBarSlot);
	}

	int32 GetMaxStackSafe(const UFortItemDefinition* Def)
	{
		if (!Def)
		{
			return 1;
		}
		const int32 MaxStack = Def->NativeGetMaxStackSize(nullptr);
		return MaxStack > 0 ? MaxStack : 1;
	}

	// Computes how many units of Def can stack onto existing WorldInventory entries without consuming a new slot.
	int32 ComputeExistingStackHeadroom(AFortPlayerController* Controller, UFortItemDefinition* Def, int32 Level, float Durability)
	{
		if (!Controller || !Controller->WorldInventory || !Def)
		{
			return 0;
		}
		const int32 MaxStack = GetMaxStackSafe(Def);
		if (MaxStack <= 1)
		{
			return 0;
		}
		int64 Headroom = 0;
		for (UFortWorldItem* Existing : Controller->WorldInventory->Inventory.ItemInstances)
		{
			if (!Existing || Existing->ItemEntry.ItemDefinition != Def)
			{
				continue;
			}
			if (FMath::Max(1, Existing->ItemEntry.Level) == Level && Existing->ItemEntry.Durability == Durability && Existing->ItemEntry.Count < MaxStack)
			{
				Headroom += static_cast<int64>(MaxStack) - Existing->ItemEntry.Count;
			}
			if (!Def->bAllowMultipleStacks)
			{
				// GiveItem stops at the first matching stack for single-stack items.
				break;
			}
		}
		return static_cast<int32>(FMath::Min<int64>(Headroom, MAX_int32));
	}

	bool IsGuidDroppablePrimary(AFortPlayerController* Controller, const FPlayerQuickBarState& State,
		const FGuid& Guid, int32& OutSlot)
	{
		OutSlot = INDEX_NONE;
		if (!Controller || !Controller->WorldInventory || !Guid.IsValid())
		{
			return false;
		}
		const FSlotLocation Loc = FindItemSlot(State, Guid);
		if (Loc.Bar != EFortQuickBars::Primary || Loc.Slot < 1 || Loc.Slot >= State.PrimarySlots.Num())
		{
			return false;
		}
		UFortWorldItem* Item = Controller->WorldInventory->NativeFindItemByGuid(Guid);
		if (!Item || Item->ItemEntry.Count <= 0)
		{
			return false;
		}
		const UFortWorldItemDefinition* WorldDef = Cast<UFortWorldItemDefinition>(Item->GetItemDefinitionBP());
		if (!WorldDef || !WorldDef->bCanBeDropped)
		{
			return false;
		}
		OutSlot = Loc.Slot;
		return true;
	}

	bool ResolveSwapTarget(AFortPlayerPawn* Pawn, AFortPlayerController* PC, const FPlayerQuickBarState& State,
		const FGuid* SwapGuidOpt, FGuid& OutSwapGuid, int32& OutSwapSlot)
	{
		OutSwapGuid.Invalidate();
		OutSwapSlot = INDEX_NONE;

		if (SwapGuidOpt)
		{
			if (!IsGuidDroppablePrimary(PC, State, *SwapGuidOpt, OutSwapSlot))
			{
				return false;
			}
			OutSwapGuid = *SwapGuidOpt;
			return true;
		}

		if (Pawn && Pawn->CurrentWeapon)
		{
			const FGuid HeldGuid = Pawn->CurrentWeapon->GetInventoryGUID();
			if (IsGuidDroppablePrimary(PC, State, HeldGuid, OutSwapSlot))
			{
				OutSwapGuid = HeldGuid;
				return true;
			}
		}

		return false;
	}

	void BuildTossLocations(AFortPawn* Pawn, const FVector& RequestedDir, FVector& OutStart, FVector& OutFinal, FVector& OutDir)
	{
		const FVector PawnLoc = Pawn ? Pawn->GetActorLocation() : FVector::ZeroVector;
		FVector Forward = RequestedDir.GetSafeNormal2D();
		if (Forward.IsNearlyZero() && Pawn)
		{
			if (AController* Ctrl = Pawn->GetController())
			{
				Forward = Ctrl->GetControlRotation().Vector().GetSafeNormal2D();
			}
		}
		if (Forward.IsNearlyZero() && Pawn)
		{
			Forward = Pawn->GetActorForwardVector().GetSafeNormal2D();
		}
		if (Forward.IsNearlyZero())
		{
			Forward = FVector::ForwardVector;
		}

		const float YawJitter = FMath::FRandRange(-18.0f, 18.0f);
		const FVector TossedDir = Forward.RotateAngleAxis(YawJitter, FVector::UpVector).GetSafeNormal();
		const float Distance = FMath::FRandRange(150.0f, 220.0f);

		OutStart = PawnLoc + FVector(0.0f, 0.0f, 40.0f) + TossedDir * 35.0f;
		OutFinal = PawnLoc + TossedDir * Distance;
		OutDir = TossedDir;

		if (Pawn && Pawn->GetWorld())
		{
			FHitResult Hit;
			FCollisionQueryParams Params(SCENE_QUERY_STAT(LootTransferTossGround), false, Pawn);
			const FVector TraceStart = OutFinal + FVector(0.0f, 0.0f, 100.0f);
			const FVector TraceEnd = OutFinal - FVector(0.0f, 0.0f, 500.0f);
			if (Pawn->GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, Params))
			{
				OutFinal = Hit.ImpactPoint + FVector(0.0f, 0.0f, 12.0f);
			}
		}
	}

	void InitializeSpawnedPickupActor(AFortPickup* Pickup, const FVector& InitialLocation)
	{
		if (!Pickup)
		{
			return;
		}
		if (!Pickup->GetRootComponent())
		{
			// stand-in: generated AFortPickup lacks native subobject creation in ctor; attach a scene root
			// so ActorLocation and movement replication remain valid.
			USceneComponent* Root = NewObject<USceneComponent>(Pickup, TEXT("LootTransferRoot"));
			if (Root)
			{
				Pickup->SetRootComponent(Root);
				Root->RegisterComponent();
			}
		}
		Pickup->SetActorLocation(InitialLocation);
		Pickup->SetReplicates(true);
		Pickup->SetReplicateMovement(true);
		// Native ctor defaults from 0x1420231b0 / 0x1419b7ab0.
		// The constructor value (1.5 s) made the vanilla client's pickup flight (it sends the pickup's fly time back in ServerHandlePickup)
		// read as very slow; retail loot snaps to the player in a fraction of a second.
		Pickup->DefaultFlyTime = 0.4f;
		Pickup->bClientUseInterpolationOnly = true;
		Pickup->bUsePickupWidget = true;
		Pickup->bDoServerHandlePickupTrace = true;
		Pickup->SimulatingTooLongLength = 30.0f;
		Pickup->OptionalOwnerID = INDEX_NONE;
		Pickup->CachedSpecialActorIdx = INDEX_NONE;
		Pickup->SetPickedUp(false);
	}

	// Transfers up to MaxEntryAccept units from SrcEntry into Controller, handling quickbar slot assignment
	// or full-bar swap when appropriate. OutRemaining stays >= 0.
	int32 GrantEntryToPlayer(AFortPlayerPawn* Pawn, AFortPlayerController* PC, const FFortItemEntry& SrcEntry,
		const FGuid* SwapGuidOpt, bool bAllowSwap, bool& OutPerformedSwap)
	{
		OutPerformedSwap = false;
		UFortItemDefinition* Def = GetEntryDef(SrcEntry);
		if (!Pawn || !PC || !PC->WorldInventory || !Def || SrcEntry.Count <= 0)
		{
			return 0;
		}

		FPlayerQuickBarState& State = GetOrCreateState(PC);
		PruneStaleSlotGuids(PC, State);

		EFortQuickBars TargetBar = EFortQuickBars::Max_None;
		int32 MinSlot = INDEX_NONE;
		int32 MaxSlot = INDEX_NONE;
		int32 MaxSlotsInBar = 0;
		const FQuickBarData* BarDefinition = nullptr;
		const bool bNeedsQuickBar = ResolveQuickBarRange(PC, Def, TargetBar, MinSlot, MaxSlot, MaxSlotsInBar, BarDefinition);

		const int32 Level = SrcEntry.Level > 0 ? SrcEntry.Level : 1;
		const int32 LoadedAmmo = FortAthenaInventory::NormalizeLoadedAmmo(Def, SrcEntry.LoadedAmmo);
		int32 MaxGrantable = SrcEntry.Count;

		if (bNeedsQuickBar)
		{
			TArray<FGuid>* Slots = State.GetBarArray(TargetBar);
			if (Slots && Slots->Num() < MaxSlotsInBar)
			{
				Slots->SetNum(MaxSlotsInBar);
			}

			const int32 StackHeadroom = LoadedAmmo > 0 ? 0 : ComputeExistingStackHeadroom(PC, Def, Level, SrcEntry.Durability);
			const UFortWorldItemDefinition* WorldDef = Cast<UFortWorldItemDefinition>(Def);
			const int32 Span = WorldDef ? FMath::Max(1, static_cast<int32>(WorldDef->NumberOfSlotsToTake)) : 1;
			const int32 FreeSlot = Slots ? FindFreeSlotRange(*Slots, MinSlot, MaxSlot, Span, BarDefinition, GetQuickBarItemType(Def)) : INDEX_NONE;

			if (FreeSlot == INDEX_NONE)
			{
				// Bar has no empty slot: try stacking into existing slot(s) first.
				if (StackHeadroom > 0)
				{
					MaxGrantable = FMath::Min(SrcEntry.Count, StackHeadroom);
				}
				else if (bAllowSwap && TargetBar == EFortQuickBars::Primary)
				{
					FGuid SwapGuid;
					int32 SwapSlot = INDEX_NONE;
					if (!ResolveSwapTarget(Pawn, PC, State, SwapGuidOpt, SwapGuid, SwapSlot))
					{
						UE_LOG(LogTemp, Display, TEXT("LootTransfer: Pickup rejected (primary bar full, no swappable weapon) pc=%s def=%s"),
							*PC->GetName(), *Def->GetName());
						return 0;
					}

					if (Def->IsStackable() && !Def->bAllowMultipleStacks)
					{
						const UFortWorldItem* Existing = PC->WorldInventory->NativeFindWorldItemByDefinition(Def, true);
						if (Existing && Existing->ItemEntry.ItemGuid != SwapGuid)
						{
							// The matching stack is full and would still exist after this swap.
							return 0;
						}
					}

					TArray<FGuid> SlotsAfterSwap = State.PrimarySlots;
					ClearGuidFromSlots(SlotsAfterSwap, SwapGuid);
					if (FindFreeSlotRange(SlotsAfterSwap, MinSlot, MaxSlot, Span, BarDefinition, GetQuickBarItemType(Def)) == INDEX_NONE)
					{
						return 0;
					}

					UFortWorldItem* OutgoingItem = PC->WorldInventory->NativeFindItemByGuid(SwapGuid);
					if (!OutgoingItem || OutgoingItem->ItemEntry.Count <= 0)
					{
						return 0;
					}

					const FFortItemEntry OutgoingEntry = OutgoingItem->ItemEntry;
					const FGuid PreviouslyHeldGuid = Pawn->CurrentWeapon ? Pawn->CurrentWeapon->GetInventoryGUID() : FGuid();
					const bool bWasHoldingSwapped = (PreviouslyHeldGuid == SwapGuid);

					const UFortWorldItemDefinition* OutgoingDef = Cast<UFortWorldItemDefinition>(OutgoingEntry.ItemDefinition);
					if (!OutgoingDef || !OutgoingDef->bCanBeDropped)
					{
						return 0;
					}
					AFortPickupAthena* DroppedPickup = nullptr;
					if (OutgoingDef->DropBehavior != EWorldItemDropBehavior::DestroyOnDrop)
					{
						FVector TossStart, TossFinal, TossDir;
						BuildTossLocations(Pawn, Pawn->GetActorForwardVector(), TossStart, TossFinal, TossDir);

						const uint8 SourceFlags = static_cast<uint8>(EFortPickupSourceTypeFlag::Player) |
							static_cast<uint8>(EFortPickupSourceTypeFlag::Tossed);
						DroppedPickup = FortAthenaLootTransfer::SpawnTossedPickupFromEntry(
							PC->GetWorld(), OutgoingEntry, TossStart, TossDir, Pawn, SourceFlags, EFortPickupSpawnSource::Unset);
						if (!DroppedPickup)
						{
							UE_LOG(LogTemp, Warning, TEXT("LootTransfer: Swap aborted (failed to spawn outgoing pickup) pc=%s out=%s"),
								*PC->GetName(), *OutgoingEntry.ItemGuid.ToString());
							return 0;
						}
					}

					const int32 Removed = FortAthenaInventory::RemoveItem(PC, SwapGuid, OutgoingEntry.Count);
					if (DroppedPickup)
					{
						if (Removed <= 0)
						{
							DroppedPickup->Destroy();
						}
						else if (Removed < OutgoingEntry.Count)
						{
							DroppedPickup->PrimaryPickupItemEntry.Count = Removed;
							DroppedPickup->PrimaryPickupItemEntry.LoadedAmmo = 0;
						}
					}
					if (Removed != OutgoingEntry.Count)
					{
						return 0;
					}

					int32 AcceptedCount = 0;
					UFortWorldItem* GrantedItem = FortAthenaInventory::GiveItem(
						PC, Def, SrcEntry.Count, Level, LoadedAmmo, &AcceptedCount, SwapSlot, bWasHoldingSwapped, SrcEntry.Durability);
					if (!GrantedItem || AcceptedCount <= 0)
					{
						UE_LOG(LogTemp, Warning, TEXT("LootTransfer: Swap GiveItem returned 0 after drop pc=%s def=%s"),
							*PC->GetName(), *Def->GetName());
						return 0;
					}

					const FGuid GrantedGuid = GrantedItem->NativeGetItemGuid();
					const FSlotLocation GrantedSlot = FindItemSlot(State, GrantedGuid);
					if (bWasHoldingSwapped && GrantedSlot.IsValid())
					{
						FortAthenaInventory::ExecuteInventoryItem(PC, GrantedGuid);
						PC->ClientActivateSlot(GrantedSlot.Bar, GrantedSlot.Slot, 0.0f, true, true);
					}

					OutPerformedSwap = true;
					UE_LOG(LogTemp, Display, TEXT("LootTransfer: Swapped pc=%s out=%s(%s) in=%s(%s) slot=%d accepted=%d remainder=%d"),
						*PC->GetName(),
						OutgoingEntry.ItemDefinition ? *OutgoingEntry.ItemDefinition->GetName() : TEXT("None"),
						*SwapGuid.ToString(),
						*Def->GetName(),
						*GrantedGuid.ToString(),
						GrantedSlot.Slot,
						AcceptedCount,
						SrcEntry.Count - AcceptedCount);
					return AcceptedCount;
				}
				else
				{
					UE_LOG(LogTemp, Display, TEXT("LootTransfer: Pickup rejected (bar %d full) pc=%s def=%s"),
						static_cast<int32>(TargetBar), *PC->GetName(), *Def->GetName());
					return 0;
				}
			}
		}

		// Snapshot known GUIDs before GiveItem so split-stack overflows (where GiveItem tops up an
		// existing stack AND creates a brand-new item instance) are also synced to quickbars.
		TSet<FGuid> ExistingGuids;
		for (UFortWorldItem* Existing : PC->WorldInventory->Inventory.ItemInstances)
		{
			if (Existing && Existing->ItemEntry.Count > 0)
			{
				ExistingGuids.Add(Existing->NativeGetItemGuid());
			}
		}

		int32 AcceptedCount = 0;
		UFortWorldItem* GrantedItem = FortAthenaInventory::GiveItem(
			PC, Def, MaxGrantable, Level, LoadedAmmo, &AcceptedCount, INDEX_NONE, false, SrcEntry.Durability);

		if (AcceptedCount <= 0)
		{
			return 0;
		}

		// Ensure all newly created quickbar-eligible instances receive DelayedQuickBarActions.
		bool bAnyNewSlotted = false;
		for (UFortWorldItem* Inst : PC->WorldInventory->Inventory.ItemInstances)
		{
			if (!Inst || Inst->ItemEntry.Count <= 0)
			{
				continue;
			}
			const FGuid InstGuid = Inst->NativeGetItemGuid();
			if (!ExistingGuids.Contains(InstGuid))
			{
				if (FortAthenaLootTransfer::OnItemGranted(PC, Inst, EFortQuickBars::Max_None, INDEX_NONE, false))
				{
					bAnyNewSlotted = true;
				}
			}
		}
		if (!bAnyNewSlotted && GrantedItem && !ExistingGuids.Contains(GrantedItem->NativeGetItemGuid()))
		{
			FortAthenaLootTransfer::OnItemGranted(PC, GrantedItem, EFortQuickBars::Max_None, INDEX_NONE, false);
		}

		UE_LOG(LogTemp, Display, TEXT("LootTransfer: Granted pickup pc=%s def=%s guid=%s accepted=%d remainder=%d loadedAmmo=%d"),
			*PC->GetName(),
			*Def->GetName(),
			GrantedItem ? *GrantedItem->NativeGetItemGuid().ToString() : TEXT("None"),
			AcceptedCount,
			SrcEntry.Count - AcceptedCount,
			SrcEntry.LoadedAmmo);

		return AcceptedCount;
	}

	void BeginPickupFlightAndDestroy(AFortPickup* Pickup, AFortPlayerPawn* Pawn,
		float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound)
	{
		if (!Pickup || !Pawn)
		{
			return;
		}

		const float DefaultFly = (Pickup->DefaultFlyTime > 0.0f) ? Pickup->DefaultFlyTime : 0.25f;
		const float ClampedFly = FMath::Clamp(InFlyTime > 0.0f ? InFlyTime : DefaultFly, 0.10f, 0.45f);
		const FVector PickupLoc = Pickup->GetActorLocation();
		const FVector PawnLoc = Pawn->GetActorLocation();
		FVector Dir = InStartDirection.GetSafeNormal();
		if (Dir.IsNearlyZero())
		{
			Dir = (PawnLoc - PickupLoc).GetSafeNormal();
		}

		// Drives AFortPickup::OnRep_PickupLocationData (0x142036270) & OnRep_bPickedUp (0x1420367b0)
		// so the vanilla client animates pickup flight before destruction.
		Pickup->PickupLocationData.PickupTarget = Pawn;
		Pickup->PickupLocationData.ItemOwner = Pawn;
		Pickup->PickupLocationData.LootInitialPosition = PickupLoc;
		Pickup->PickupLocationData.LootFinalPosition = PawnLoc;
		Pickup->PickupLocationData.FlyTime = ClampedFly;
		Pickup->PickupLocationData.StartDirection = Dir;
		Pickup->PickupLocationData.bPlayPickupSound = bPlayPickupSound;

		Pickup->SetPickedUp(true);
		Pickup->SetActorEnableCollision(false);
		Pickup->ForceNetUpdate();

		if (UWorld* World = Pickup->GetWorld())
		{
			FTimerHandle DestroyTimer;
			TWeakObjectPtr<AFortPickup> WeakPickup(Pickup);
			const float DestroyDelay = FMath::Clamp(ClampedFly + 0.1f, 0.25f, 0.6f);
			World->GetTimerManager().SetTimer(
				DestroyTimer,
				[WeakPickup]()
				{
					if (AFortPickup* Target = WeakPickup.Get())
					{
						if (IsValid(Target))
						{
							Target->Destroy();
						}
					}
				},
				DestroyDelay,
				false);
		}
		else
		{
			Pickup->Destroy();
		}
	}

	bool HandlePickupInternal(AFortPlayerPawn* Pawn, AFortPickup* Pickup, const FGuid* SwapGuidOpt,
		float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound, bool bExplicitSwapRpc, bool bAutomatic = false)
	{
		if (!FortAthenaLootTransfer::IsEnabled() || !Pawn || !Pawn->HasAuthority() || Pawn->IsDead() || Pawn->bIsDBNO)
		{
			return false;
		}
		AFortPlayerController* PC = Cast<AFortPlayerController>(Pawn->GetController());
		if (!PC || !PC->WorldInventory || !PC->WorldInventory->HasAuthority() ||
			!IsValid(Pickup) || Pickup->GetWorld() != Pawn->GetWorld() || Pickup->bPickedUp)
		{
			return false;
		}

		if (Pickup->bOnlyRelevantToOwner && Pickup->GetOwner() != PC)
		{
			return false;
		}

		if (Pickup->GetRootComponent())
		{
			const float DistSq = FVector::DistSquared(Pawn->GetActorLocation(), Pickup->GetActorLocation());
			if (DistSq > MaxPickupRangeSq)
			{
				UE_LOG(LogTemp, Warning, TEXT("LootTransfer: Pickup rejected (out of range dist=%.1f pawn=%s pickup=%s) pc=%s pickup=%s"),
					FMath::Sqrt(DistSq), *Pawn->GetActorLocation().ToString(), *Pickup->GetActorLocation().ToString(), *PC->GetName(), *Pickup->GetName());
				return false;
			}
		}

		// Inventory changes can invoke callbacks before the pickup's remaining count is committed.
		if (PickupsBeingGranted.Contains(Pickup))
		{
			return false;
		}
		PickupsBeingGranted.Add(Pickup);
		ON_SCOPE_EXIT { PickupsBeingGranted.Remove(Pickup); };

		// Ensure existing inventory items (e.g. SetupPlayer pickaxe & building pieces) are indexed first.
		FortAthenaLootTransfer::SetupPlayerQuickBars(PC);

		int64 TotalAccepted = 0;
		if (Pickup->MultiItemPickupEntries.Num() > 0)
		{
			bool bSwapUsed = false;
			for (int32 i = Pickup->MultiItemPickupEntries.Num() - 1; i >= 0; --i)
			{
				FFortItemEntry& Entry = Pickup->MultiItemPickupEntries[i];
				if (!Entry.ItemDefinition || Entry.Count <= 0)
				{
					continue;
				}
				bool bDidSwap = false;
				const bool bCanSwapHere = !bAutomatic && !bSwapUsed && (bExplicitSwapRpc || i == 0);
				const int32 Accepted = GrantEntryToPlayer(Pawn, PC, Entry, SwapGuidOpt, bCanSwapHere, bDidSwap);
				if (bDidSwap)
				{
					bSwapUsed = true;
				}
				if (Accepted > 0)
				{
					TotalAccepted += Accepted;
					Entry.Count -= Accepted;
					Entry.LoadedAmmo = 0; // The grant already used the entry's magazine.
					if (Entry.Count <= 0)
					{
						Pickup->MultiItemPickupEntries.RemoveAt(i);
					}
				}
			}

			if (TotalAccepted <= 0)
			{
				return false;
			}

			// Keep rejected bundles intact so a retry cannot grant their display entry as a standalone item.
			Pickup->MultiItemPickupEntries.RemoveAll([](const FFortItemEntry& Entry)
			{
				return !Entry.ItemDefinition || Entry.Count <= 0;
			});
			if (Pickup->MultiItemPickupEntries.Num() > 0)
			{
				Pickup->PrimaryPickupItemEntry = Pickup->MultiItemPickupEntries[0];
				Pickup->ForceNetUpdate();
				UE_LOG(LogTemp, Display, TEXT("LootTransfer: Multi-pickup partial accept pc=%s pickup=%s accepted=%lld remainingEntries=%d"),
					*PC->GetName(), *Pickup->GetName(), TotalAccepted, Pickup->MultiItemPickupEntries.Num());
				return true;
			}

			Pickup->PrimaryPickupItemEntry.Count = 0;
			BeginPickupFlightAndDestroy(Pickup, Pawn, InFlyTime, InStartDirection, bPlayPickupSound);
			UE_LOG(LogTemp, Display, TEXT("LootTransfer: Multi-pickup completed pc=%s pickup=%s accepted=%lld"),
				*PC->GetName(), *Pickup->GetName(), TotalAccepted);
			return true;
		}

		if (!Pickup->PrimaryPickupItemEntry.ItemDefinition || Pickup->PrimaryPickupItemEntry.Count <= 0)
		{
			return false;
		}

		bool bDidSwap = false;
		const int32 Accepted = GrantEntryToPlayer(
			Pawn, PC, Pickup->PrimaryPickupItemEntry, SwapGuidOpt, !bAutomatic, bDidSwap);
		if (Accepted <= 0)
		{
			return false;
		}

		Pickup->PrimaryPickupItemEntry.Count -= Accepted;
		if (Pickup->PrimaryPickupItemEntry.Count > 0)
		{
			Pickup->PrimaryPickupItemEntry.LoadedAmmo = 0; // Keep the magazine with the first grant.
			Pickup->ForceNetUpdate();
			UE_LOG(LogTemp, Display, TEXT("LootTransfer: Partial pickup pc=%s pickup=%s accepted=%d remainder=%d"),
				*PC->GetName(), *Pickup->GetName(), Accepted, Pickup->PrimaryPickupItemEntry.Count);
			return true;
		}

		BeginPickupFlightAndDestroy(Pickup, Pawn, InFlyTime, InStartDirection, bPlayPickupSound);
		return true;
	}

	// Exec thunks matching shipping signatures in AFortPlayerPawn (0x142a7ed80, 0x142a7ef10, 0x142a7f100)
	// and AFortPlayerController (0x142a4a130, 0x142a3e570).
	void ExecTransferServerHandlePickup(UObject* Context, FFrame& Stack, RESULT_DECL)
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

	void ExecServerHandlePickupWithRequestedSwap(UObject* Context, FFrame& Stack, RESULT_DECL)
	{
		P_GET_OBJECT(AFortPickup, Pickup);
		P_GET_STRUCT(FGuid, Swap);
		P_GET_PROPERTY(FFloatProperty, FlyTime);
		P_GET_STRUCT(FVector, StartDirection);
		P_GET_UBOOL(bPlayPickupSound);
		P_FINISH;
		P_NATIVE_BEGIN;
		FortAthenaLootTransfer::HandlePickupWithSwap(
			Cast<AFortPlayerPawn>(Context), Pickup, Swap, FlyTime, StartDirection, bPlayPickupSound, true);
		P_NATIVE_END;
	}

	void ExecServerHandlePickupWithSwap(UObject* Context, FFrame& Stack, RESULT_DECL)
	{
		P_GET_OBJECT(AFortPickup, Pickup);
		P_GET_STRUCT(FGuid, Swap);
		P_GET_PROPERTY(FFloatProperty, FlyTime);
		P_GET_STRUCT(FVector, StartDirection);
		P_GET_UBOOL(bPlayPickupSound);
		P_FINISH;
		P_NATIVE_BEGIN;
		FortAthenaLootTransfer::HandlePickupWithSwap(
			Cast<AFortPlayerPawn>(Context), Pickup, Swap, FlyTime, StartDirection, bPlayPickupSound, false);
		P_NATIVE_END;
	}

	void ExecServerAttemptInventoryDrop(UObject* Context, FFrame& Stack, RESULT_DECL)
	{
		P_GET_STRUCT(FGuid, ItemGuid);
		P_GET_PROPERTY(FIntProperty, Count);
		P_GET_UBOOL(bTrash);
		P_FINISH;
		P_NATIVE_BEGIN;
		FortAthenaLootTransfer::AttemptInventoryDrop(Cast<AFortPlayerController>(Context), ItemGuid, Count, bTrash);
		P_NATIVE_END;
	}

	void ExecServerAcknowledgeDelayedQuickBarAction(UObject* Context, FFrame& Stack, RESULT_DECL)
	{
		P_GET_TARRAY(uint32, ProcessedActionIds);
		P_FINISH;
		P_NATIVE_BEGIN;
		FortAthenaLootTransfer::AcknowledgeDelayedQuickBarActions(
			Cast<AFortPlayerController>(Context), ProcessedActionIds);
		P_NATIVE_END;
	}

	void RebindRpc(UClass* Class, const TCHAR* Name, FNativeFuncPtr Func)
	{
		if (!Class || !Func)
		{
			return;
		}
		if (UFunction* Function = Class->FindFunctionByName(Name))
		{
			Function->SetNativeFunc(Func);
			UE_LOG(LogTemp, Display, TEXT("LootTransfer: Rebound %s::%s"), *Class->GetName(), Name);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("LootTransfer: Missing UFunction %s::%s"), *Class->GetName(), Name);
		}
	}
}

bool FortAthenaLootTransfer::IsEnabled()
{
	return FParse::Param(FCommandLine::Get(), TEXT("FortniteOriginalClient"));
}

void FortAthenaLootTransfer::InstallRpcOverrides()
{
	if (!IsEnabled())
	{
		return;
	}
	static bool bInstalled = false;
	if (bInstalled)
	{
		return;
	}
	bInstalled = true;

	RebindRpc(AFortPlayerPawn::StaticClass(), TEXT("ServerHandlePickup"), &ExecTransferServerHandlePickup);
	RebindRpc(AFortPlayerPawn::StaticClass(), TEXT("ServerHandlePickupWithRequestedSwap"), &ExecServerHandlePickupWithRequestedSwap);
	RebindRpc(AFortPlayerPawn::StaticClass(), TEXT("ServerHandlePickupWithSwap"), &ExecServerHandlePickupWithSwap);
	RebindRpc(AFortPlayerController::StaticClass(), TEXT("ServerAttemptInventoryDrop"), &ExecServerAttemptInventoryDrop);
	RebindRpc(AFortPlayerController::StaticClass(), TEXT("ServerAcknowledgeDelayedQuickBarAction"), &ExecServerAcknowledgeDelayedQuickBarAction);
}

void FortAthenaLootTransfer::SetupPlayerQuickBars(AFortPlayerController* Controller)
{
	if (!IsEnabled() || !Controller || !Controller->HasAuthority() || !Controller->WorldInventory)
	{
		return;
	}
	FPlayerQuickBarState& State = GetOrCreateState(Controller);
	PruneStaleSlotGuids(Controller, State);

	for (UFortWorldItem* Item : Controller->WorldInventory->Inventory.ItemInstances)
	{
		if (!Item || Item->ItemEntry.Count <= 0)
		{
			continue;
		}
		if (!FindItemSlot(State, Item->NativeGetItemGuid()).IsValid())
		{
			OnItemGranted(Controller, Item, EFortQuickBars::Max_None, INDEX_NONE, false);
		}
	}
}

bool FortAthenaLootTransfer::OnItemGranted(AFortPlayerController* Controller, UFortWorldItem* Item,
	EFortQuickBars PreferredBar, int32 PreferredSlot, bool bActivateSlot)
{
	if (!IsEnabled() || !Controller || !Controller->HasAuthority() || !Item || Item->ItemEntry.Count <= 0)
	{
		return false;
	}
	UFortItemDefinition* Def = Item->GetItemDefinitionBP();
	const FGuid Guid = Item->NativeGetItemGuid();
	if (!Def || !Guid.IsValid())
	{
		return false;
	}

	FPlayerQuickBarState& State = GetOrCreateState(Controller);
	PruneStaleSlotGuids(Controller, State);

	if (FindItemSlot(State, Guid).IsValid())
	{
		return true;
	}

	EFortQuickBars TargetBar = EFortQuickBars::Max_None;
	int32 MinSlot = INDEX_NONE;
	int32 MaxSlot = INDEX_NONE;
	int32 MaxSlotsInBar = 0;
	const FQuickBarData* BarDefinition = nullptr;
	if (!ResolveQuickBarRange(Controller, Def, TargetBar, MinSlot, MaxSlot, MaxSlotsInBar, BarDefinition))
	{
		// Non-quickbar item (ammo, resource, ingredient, edit tool) remains in WorldInventory only.
		return false;
	}

	TArray<FGuid>* Slots = State.GetBarArray(TargetBar);
	if (!Slots)
	{
		return false;
	}
	if (Slots->Num() < MaxSlotsInBar)
	{
		Slots->SetNum(MaxSlotsInBar);
	}

	const int32 Span = GetItemSlotSpan(Item);
	int32 ChosenSlot = INDEX_NONE;
	if (PreferredBar == TargetBar && PreferredSlot >= MinSlot && PreferredSlot <= MaxSlot - Span + 1)
	{
		ChosenSlot = FindFreeSlotRange(*Slots, PreferredSlot, PreferredSlot + Span - 1, Span, BarDefinition, GetQuickBarItemType(Def));
	}

	if (ChosenSlot == INDEX_NONE)
	{
		ChosenSlot = FindFreeSlotRange(*Slots, MinSlot, MaxSlot, Span, BarDefinition, GetQuickBarItemType(Def));
	}
	if (ChosenSlot == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("LootTransfer: No free quickbar slot for pc=%s def=%s bar=%d"),
			*Controller->GetName(), *Def->GetName(), static_cast<int32>(TargetBar));
		return false;
	}

	OccupySlots(*Slots, ChosenSlot, Span, Guid);
	EnqueueDelayedQuickBarAction(Controller, State, EFortDelayedQuickBarAction::Add,
		Guid, Def, TargetBar, ChosenSlot, bActivateSlot);

	UE_LOG(LogTemp, Display, TEXT("LootTransfer: Assigned quickbar slot pc=%s def=%s guid=%s bar=%d slot=%d span=%d"),
		*Controller->GetName(), *Def->GetName(), *Guid.ToString(),
		static_cast<int32>(TargetBar), ChosenSlot, Span);
	return true;
}

int32 FortAthenaLootTransfer::FindReplacementPrimarySlot(AFortPlayerController* Controller, const FGuid& ItemGuid,
	const UFortWorldItemDefinition* Replacement)
{
	SetupPlayerQuickBars(Controller);
	const FPlayerQuickBarState* State = GQuickBarStates.Find(Controller);
	const FSlotLocation Existing = State ? FindItemSlot(*State, ItemGuid) : FSlotLocation();
	if (!Existing.IsValid() || Existing.Bar != EFortQuickBars::Primary)
	{
		return INDEX_NONE;
	}

	EFortQuickBars TargetBar = EFortQuickBars::Max_None;
	int32 MinSlot = INDEX_NONE;
	int32 MaxSlot = INDEX_NONE;
	int32 MaxSlotsInBar = 0;
	const FQuickBarData* BarDefinition = nullptr;
	const int32 Span = FMath::Max(1, static_cast<int32>(Replacement->NumberOfSlotsToTake));
	if (!ResolveQuickBarRange(Controller, Replacement, TargetBar, MinSlot, MaxSlot, MaxSlotsInBar, BarDefinition) ||
		TargetBar != Existing.Bar || Existing.Slot < MinSlot || Existing.Slot > MaxSlot - Span + 1)
	{
		return INDEX_NONE;
	}
	TArray<FGuid> Slots = State->PrimarySlots;
	ClearGuidFromSlots(Slots, ItemGuid);
	return FindFreeSlotRange(Slots, Existing.Slot, Existing.Slot + Span - 1, Span, BarDefinition, GetQuickBarItemType(Replacement));
}

void FortAthenaLootTransfer::OnItemRemoved(AFortPlayerController* Controller, const FGuid& ItemGuid)
{
	if (!IsEnabled() || !Controller || !Controller->HasAuthority() || !ItemGuid.IsValid())
	{
		return;
	}

	FPlayerQuickBarState* State = GQuickBarStates.Find(Controller);
	if (!State)
	{
		return;
	}

	const FSlotLocation Loc = FindItemSlot(*State, ItemGuid);
	if (!Loc.IsValid())
	{
		return;
	}

	UFortItemDefinition* Def = nullptr;
	if (Controller->WorldInventory)
	{
		if (UFortWorldItem* Existing = Controller->WorldInventory->NativeFindItemByGuid(ItemGuid))
		{
			Def = Existing->GetItemDefinitionBP();
		}
	}

	if (TArray<FGuid>* Slots = State->GetBarArray(Loc.Bar))
	{
		ClearGuidFromSlots(*Slots, ItemGuid);
	}

	if (Def)
	{
		EnqueueDelayedQuickBarAction(Controller, *State, EFortDelayedQuickBarAction::Remove,
			ItemGuid, Def, Loc.Bar, Loc.Slot, true);
	}
	else
	{
		// Fallback when UFortWorldItem was already destroyed before OnItemRemoved ran:
		// send Remove directly with cached ItemGuid so client quickbar clears the slot.
		FDelayedQuickBarAction Action;
		Action.ActionId = ++State->NextActionId;
		Action.Action = EFortDelayedQuickBarAction::Remove;
		Action.ItemGuid = ItemGuid;
		Action.QuickBarType = Loc.Bar;
		Action.QuickBarSlot = Loc.Slot;
		Action.bForceExecution = true;
		Action.ReplicationID = INDEX_NONE;
		Action.ReplicationKey = INDEX_NONE;
		const int32 NewIdx = Controller->DelayedQuickBarActions.Items.Add(Action);
		Controller->DelayedQuickBarActions.MarkItemDirty(Controller->DelayedQuickBarActions.Items[NewIdx]);
		Controller->ForceNetUpdate();
	}

	UE_LOG(LogTemp, Display, TEXT("LootTransfer: Removed quickbar slot pc=%s guid=%s bar=%d slot=%d"),
		*Controller->GetName(), *ItemGuid.ToString(), static_cast<int32>(Loc.Bar), Loc.Slot);
}

void FortAthenaLootTransfer::BlockAutoPickup(AFortPickup* Pickup)
{
	if (Pickup && Pickup->HasAuthority())
	{
		BlockedAutoPickups.Add(Pickup);
	}
}

bool FortAthenaLootTransfer::TryAutoPickup(AFortPlayerPawn* Pawn, AFortPickup* Pickup)
{
	if (!IsValid(Pawn) || !IsValid(Pickup) || Pickup->GetWorld() != Pawn->GetWorld() || Pickup->bPickedUp ||
		Pickup->PickupLocationData.TossState == EFortPickupTossState::InProgress ||
		Pickup->PawnWhoDroppedPickup == Pawn || BlockedAutoPickups.Contains(Pickup) ||
		Pickup->MultiItemPickupEntries.Num() > 0 ||
		FVector::DistSquared(Pawn->GetActorLocation(), Pickup->GetActorLocation()) > AutoPickupRangeSq)
	{
		return false;
	}

	const UFortItemDefinition* Definition = Pickup->PrimaryPickupItemEntry.ItemDefinition;
	if (!Definition)
	{
		return false;
	}
	const EFortItemType Type = GetQuickBarItemType(Definition);
	if (Type != EFortItemType::Ammo && Type != EFortItemType::WorldResource && !Definition->bForceAutoPickup)
	{
		return false;
	}
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(AutoPickup), false, Pawn);
	TraceParams.AddIgnoredActor(Pickup);
	if (Pawn->GetWorld()->LineTraceTestByChannel(Pawn->GetActorLocation(), Pickup->GetActorLocation(),
		ECC_Visibility, TraceParams))
	{
		return false;
	}
	return HandlePickupInternal(Pawn, Pickup, nullptr, 0.0f, FVector::ZeroVector, true, false, true);
}

void FortAthenaLootTransfer::TickAutoPickup(UWorld* World)
{
	if (!World || !IsEnabled())
	{
		return;
	}
	static TWeakObjectPtr<UWorld> LastWorld;
	static float NextSweepTime = 0.0f;
	const float Now = World->GetTimeSeconds();
	if (LastWorld.Get() != World)
	{
		LastWorld = World;
		NextSweepTime = Now;
	}
	if (Now < NextSweepTime)
	{
		return;
	}
	NextSweepTime = Now + 0.2f;
	for (auto It = BlockedAutoPickups.CreateIterator(); It; ++It)
	{
		if (!(*It).IsValid())
		{
			It.RemoveCurrent();
		}
	}

	TArray<AFortPlayerPawn*> Players;
	for (auto It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* Controller = It->Get();
		if (AFortPlayerPawn* Pawn = Controller ? Cast<AFortPlayerPawn>(Controller->GetPawn()) : nullptr)
		{
			if (Pawn->HasAuthority() && !Pawn->IsDead() && !Pawn->bIsDBNO)
			{
				Players.Add(Pawn);
			}
		}
	}
	if (Players.Num() == 0)
	{
		return;
	}

	for (TActorIterator<AFortPickup> It(World); It; ++It)
	{
		for (AFortPlayerPawn* Pawn : Players)
		{
			TryAutoPickup(Pawn, *It);
		}
	}
}

bool FortAthenaLootTransfer::HandlePickup(AFortPlayerPawn* Pawn, AFortPickup* Pickup,
	float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound)
{
	return HandlePickupInternal(Pawn, Pickup, nullptr, InFlyTime, InStartDirection, bPlayPickupSound, false);
}

bool FortAthenaLootTransfer::HandlePickupWithSwap(AFortPlayerPawn* Pawn, AFortPickup* Pickup,
	const FGuid& SwapGuid, float InFlyTime, const FVector& InStartDirection,
	bool bPlayPickupSound, bool bRequestedSwap)
{
	UE_LOG(LogTemp, Display, TEXT("LootTransfer: HandlePickupWith%sSwap pawn=%s pickup=%s swapGuid=%s"),
		bRequestedSwap ? TEXT("Requested") : TEXT(""),
		Pawn ? *Pawn->GetName() : TEXT("None"),
		Pickup ? *Pickup->GetName() : TEXT("None"),
		*SwapGuid.ToString());
	return HandlePickupInternal(Pawn, Pickup, &SwapGuid, InFlyTime, InStartDirection, bPlayPickupSound, true);
}

bool FortAthenaLootTransfer::AttemptInventoryDrop(AFortPlayerController* Controller, const FGuid& ItemGuid,
	int32 Count, bool bTrash)
{
	if (!IsEnabled() || !Controller || !Controller->HasAuthority() || !Controller->WorldInventory
		|| !ItemGuid.IsValid() || Count <= 0)
	{
		return false;
	}

	SetupPlayerQuickBars(Controller);

	UFortWorldItem* Item = Controller->WorldInventory->NativeFindItemByGuid(ItemGuid);
	if (!Item || Item->ItemEntry.Count <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("LootTransfer: Drop rejected (item not found) pc=%s guid=%s"),
			*Controller->GetName(), *ItemGuid.ToString());
		return false;
	}

	UFortItemDefinition* Def = Item->GetItemDefinitionBP();
	UFortWorldItemDefinition* WorldDef = Cast<UFortWorldItemDefinition>(Def);
	if (!Def || !WorldDef || !WorldDef->bCanBeDropped)
	{
		UE_LOG(LogTemp, Warning, TEXT("LootTransfer: Drop rejected (not droppable) pc=%s def=%s guid=%s"),
			*Controller->GetName(), Def ? *Def->GetName() : TEXT("None"), *ItemGuid.ToString());
		return false;
	}

	const int32 DropCount = FMath::Min(Count, Item->ItemEntry.Count);
	const bool bDroppingEntireStack = (DropCount >= Item->ItemEntry.Count);
	const bool bDestroyOnly = bTrash || (WorldDef->DropBehavior == EWorldItemDropBehavior::DestroyOnDrop);

	AFortPawn* FortPawn = Cast<AFortPawn>(Controller->GetPawn());
	const bool bWasHoldingItem = (FortPawn && FortPawn->CurrentWeapon && FortPawn->CurrentWeapon->GetInventoryGUID() == ItemGuid);

	FFortItemEntry OutgoingEntry = Item->ItemEntry;
	OutgoingEntry.Count = DropCount;
	if (!bDroppingEntireStack)
	{
		// Avoid duplicating loaded magazine ammo when splitting a stack.
		OutgoingEntry.LoadedAmmo = 0;
	}

	AFortPickupAthena* SpawnedPickup = nullptr;
	if (!bDestroyOnly)
	{
		if (!FortPawn)
		{
			UE_LOG(LogTemp, Warning, TEXT("LootTransfer: Drop aborted (no pawn) pc=%s guid=%s"),
				*Controller->GetName(), *ItemGuid.ToString());
			return false;
		}
		FVector TossStart, TossFinal, TossDir;
		BuildTossLocations(FortPawn, FVector::ZeroVector, TossStart, TossFinal, TossDir);
		const uint8 SourceFlags = static_cast<uint8>(EFortPickupSourceTypeFlag::Player) |
			static_cast<uint8>(EFortPickupSourceTypeFlag::Tossed);
		SpawnedPickup = SpawnTossedPickupFromEntry(
			Controller->GetWorld(), OutgoingEntry, TossStart, TossDir, FortPawn, SourceFlags, EFortPickupSpawnSource::Unset);
		if (!SpawnedPickup)
		{
			UE_LOG(LogTemp, Warning, TEXT("LootTransfer: Drop aborted (failed to spawn pickup) pc=%s def=%s guid=%s"),
				*Controller->GetName(), *Def->GetName(), *ItemGuid.ToString());
			return false;
		}
	}

	const int32 Removed = FortAthenaInventory::RemoveItem(Controller, ItemGuid, DropCount);
	if (Removed <= 0)
	{
		if (SpawnedPickup)
		{
			SpawnedPickup->Destroy();
		}
		return false;
	}
	if (SpawnedPickup)
	{
		SpawnedPickup->PrimaryPickupItemEntry.Count = Removed;
		if (Removed < DropCount)
		{
			SpawnedPickup->PrimaryPickupItemEntry.LoadedAmmo = 0;
		}
	}

	FPlayerQuickBarState& State = GetOrCreateState(Controller);
	UFortWorldItem* RemainingItem = Controller->WorldInventory->NativeFindItemByGuid(ItemGuid);
	const bool bNowGone = (!RemainingItem || RemainingItem->ItemEntry.Count <= 0);

	if (bDroppingEntireStack && bNowGone && bWasHoldingItem && State.PrimarySlots.IsValidIndex(0) && State.PrimarySlots[0].IsValid())
	{
		FortAthenaInventory::ExecuteInventoryItem(Controller, State.PrimarySlots[0]);
		Controller->ClientActivateSlot(EFortQuickBars::Primary, 0, 0.0f, true, true);
	}

	UE_LOG(LogTemp, Display, TEXT("LootTransfer: Dropped pc=%s def=%s guid=%s requested=%d removed=%d trash=%d pickup=%s loadedAmmo=%d"),
		*Controller->GetName(),
		*Def->GetName(),
		*ItemGuid.ToString(),
		Count,
		Removed,
		bDestroyOnly ? 1 : 0,
		SpawnedPickup ? *SpawnedPickup->GetName() : TEXT("None"),
		OutgoingEntry.LoadedAmmo);

	return true;
}

void FortAthenaLootTransfer::AcknowledgeDelayedQuickBarActions(AFortPlayerController* Controller,
	const TArray<uint32>& ProcessedActionIds)
{
	if (!IsEnabled() || !Controller || !Controller->HasAuthority() || ProcessedActionIds.Num() <= 0)
	{
		return;
	}

	bool bRemovedAny = false;
	for (int32 i = Controller->DelayedQuickBarActions.Items.Num() - 1; i >= 0; --i)
	{
		if (ProcessedActionIds.Contains(Controller->DelayedQuickBarActions.Items[i].ActionId))
		{
			Controller->DelayedQuickBarActions.Items.RemoveAtSwap(i);
			bRemovedAny = true;
		}
	}
	if (bRemovedAny)
	{
		Controller->DelayedQuickBarActions.MarkArrayDirty();
		UE_LOG(LogTemp, Display, TEXT("LootTransfer: Acked %d delayed quickbar actions pc=%s remaining=%d"),
			ProcessedActionIds.Num(), *Controller->GetName(), Controller->DelayedQuickBarActions.Items.Num());
	}
}

void FortAthenaLootTransfer::InitializeWorldPickup(AFortPickup* Pickup, const FVector& Location)
{
	InitializeSpawnedPickupActor(Pickup, Location);
	// The pickup's projectile component has gravity on and nothing to collide with (its root is a plain scene component), so a
	// resting loot pickup would fall away from where it was placed (the server rejected the client's pickup requests as out of
	// range at 1971 then 2818 units, growing). World loot is placed by the toss data, not by physics.
	if (Pickup)
	{
		if (UProjectileMovementComponent* Movement = Pickup->FindComponentByClass<UProjectileMovementComponent>())
		{
			Movement->StopMovementImmediately();
			Movement->Deactivate();
			Movement->SetComponentTickEnabled(false);
		}
	}
}

void FortAthenaLootTransfer::BeginServerToss(AFortPickup* Pickup, const FVector& Start, const FVector& Final, float FlyTime)
{
	UWorld* World = Pickup ? Pickup->GetWorld() : nullptr;
	if (!World || !Pickup->GetRootComponent())
	{
		return;
	}
	FlyTime = FMath::Max(FlyTime, 0.1f);
	UProjectileMovementComponent* Movement = Pickup->MovementComponent ? Pickup->MovementComponent : Pickup->FindComponentByClass<UProjectileMovementComponent>();
	if (Movement)
	{
		Pickup->SetActorLocation(Start);
		Movement->SetUpdatedComponent(Pickup->GetRootComponent());
		Movement->bShouldBounce = false;
		Movement->bRotationFollowsVelocity = false;
		Movement->bInitialVelocityInLocalSpace = false;
		Movement->ProjectileGravityScale = 1.5f;
		const float Gz = Movement->GetGravityZ(); // already scaled, negative
		// Ballistic launch velocity that lands on Final after exactly FlyTime: x = x0 + v*t + g*t^2/2.
		Movement->Velocity = (Final - Start) / FlyTime - FVector(0.0f, 0.0f, 0.5f * Gz * FlyTime);
		Movement->SetComponentTickEnabled(true);
		Movement->Activate(true);
		Movement->UpdateComponentVelocity();
		Pickup->NetUpdateFrequency = 30.0f;
		Pickup->ForceNetUpdate();
	}

	FTimerHandle RestTimer;
	TWeakObjectPtr<AFortPickup> WeakPickup(Pickup);
	World->GetTimerManager().SetTimer(RestTimer, [WeakPickup, Final]()
	{
		AFortPickup* Target = WeakPickup.Get();
		if (!IsValid(Target) || Target->bPickedUp)
		{
			return;
		}
		if (UProjectileMovementComponent* M = Target->MovementComponent ? Target->MovementComponent : Target->FindComponentByClass<UProjectileMovementComponent>())
		{
			M->StopMovementImmediately();
			M->Deactivate();
			M->SetComponentTickEnabled(false);
		}
		Target->SetActorLocation(Final);
		Target->NetUpdateFrequency = 10.0f;
		Target->PickupLocationData.TossState = EFortPickupTossState::AtRest;
		Target->ForceNetUpdate();
	}, FlyTime, false);
}

AFortPickupAthena* FortAthenaLootTransfer::SpawnTossedPickupFromEntry(UWorld* World, const FFortItemEntry& Entry,
	const FVector& StartLocation, const FVector& TossDirection, AFortPawn* OwnerPawn,
	uint8 SourceTypeFlags, EFortPickupSpawnSource SpawnSource, float FlyTime)
{
	if (!IsEnabled() || !World || World->GetNetMode() == NM_Client || !Entry.ItemDefinition || Entry.Count <= 0)
	{
		return nullptr;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.Owner = OwnerPawn;
	AFortPickupAthena* Pickup = World->SpawnActor<AFortPickupAthena>(StartLocation, FRotator::ZeroRotator, Params);
	if (!Pickup)
	{
		return nullptr;
	}

	InitializeSpawnedPickupActor(Pickup, StartLocation);

	Pickup->PrimaryPickupItemEntry = Entry;
	Pickup->PrimaryPickupItemEntry.LoadedAmmo = FortAthenaInventory::NormalizeLoadedAmmo(Entry.ItemDefinition, Entry.LoadedAmmo);
	Pickup->PrimaryPickupItemEntry.ItemGuid = FGuid::NewGuid();
	Pickup->PrimaryPickupItemEntry.bIsReplicatedCopy = false;
	Pickup->PrimaryPickupItemEntry.bIsDirty = false;
	Pickup->PrimaryPickupItemEntry.ParentInventory.Reset();

	Pickup->PawnWhoDroppedPickup = OwnerPawn;
	Pickup->PickupSourceTypeFlags = static_cast<EFortPickupSourceTypeFlag>(SourceTypeFlags);
	Pickup->PickupSpawnSource = SpawnSource;
	Pickup->bTossedFromContainer = false;

	FVector Dir = TossDirection.GetSafeNormal();
	if (Dir.IsNearlyZero())
	{
		Dir = FVector::ForwardVector;
	}
	FVector TossFinal = StartLocation + Dir * 180.0f;
	if (OwnerPawn)
	{
		FVector UnusedStart, ComputedDir;
		BuildTossLocations(OwnerPawn, Dir, UnusedStart, TossFinal, ComputedDir);
		Dir = ComputedDir;
	}

	const float ResolvedFlyTime = (FlyTime > 0.0f) ? FMath::Clamp(FlyTime, 0.25f, 1.50f) : 0.75f;

	// Reconstruct TossPickup (0x14203cda0 -> 0x142029370 -> 0x142039830):
	// Replicate Initial/Final positions and InProgress state so clients interpolate the arc cleanly,
	// then settle server location + AtRest state when flight finishes.
	Pickup->PickupLocationData.ItemOwner = OwnerPawn;
	Pickup->PickupLocationData.PickupTarget = nullptr;
	Pickup->PickupLocationData.CombineTarget = nullptr;
	Pickup->PickupLocationData.LootInitialPosition = StartLocation;
	Pickup->PickupLocationData.LootFinalPosition = TossFinal;
	Pickup->PickupLocationData.FinalTossRestLocation = TossFinal;
	Pickup->PickupLocationData.FlyTime = ResolvedFlyTime;
	Pickup->PickupLocationData.StartDirection = Dir;
	Pickup->PickupLocationData.TossState = EFortPickupTossState::InProgress;
	Pickup->PickupLocationData.bPlayPickupSound = false;
	Pickup->PickupLocationData.PickupGuid = FGuid::NewGuid();
	Pickup->ForceNetUpdate();

	BeginServerToss(Pickup, StartLocation, TossFinal, ResolvedFlyTime);

	UE_LOG(LogTemp, Display, TEXT("LootTransfer: Spawned tossed pickup %s def=%s count=%d loadedAmmo=%d flyTime=%.2f"),
		*Pickup->GetName(),
		*Entry.ItemDefinition->GetName(),
		Entry.Count,
		Pickup->PrimaryPickupItemEntry.LoadedAmmo,
		ResolvedFlyTime);

	return Pickup;
}

int32 FortAthenaLootTransfer::DropAllOnDeath(AFortPlayerController* VictimController, AFortPawn* VictimPawn)
{
	AFortInventory* Inventory = VictimController ? VictimController->WorldInventory : nullptr;
	UWorld* World = VictimPawn ? VictimPawn->GetWorld() : nullptr;
	if (!Inventory || !World || !VictimController->HasAuthority() || !VictimPawn->HasAuthority())
	{
		return 0;
	}
	if (const AFortPlayerPawnAthena* AthenaPawn = Cast<AFortPlayerPawnAthena>(VictimPawn))
	{
		if (!AthenaPawn->bShouldDropItemsOnDeath)
		{
			return 0;
		}
	}
	// stand-in: the original elimination drop (backpack pickup/spread rules) is not recovered; each droppable stack becomes one
	// pickup tossed in a different direction so the loot fans out instead of stacking on one spot.
	TArray<FGuid> ItemGuids;
	for (const UFortWorldItem* Item : Inventory->Inventory.ItemInstances)
	{
		if (Item)
		{
			ItemGuids.Add(Item->ItemEntry.ItemGuid);
		}
	}
	int32 Spawned = 0;
	const float BaseYaw = FMath::FRandRange(0.0f, 360.0f);
	for (const FGuid& ItemGuid : ItemGuids)
	{
		const UFortWorldItem* Item = Inventory->NativeFindItemByGuid(ItemGuid);
		UFortWorldItemDefinition* Def = Item ? Cast<UFortWorldItemDefinition>(Item->ItemEntry.ItemDefinition) : nullptr;
		if (!Def || !Def->bCanBeDropped || Item->ItemEntry.Count <= 0)
		{
			continue;
		}
		if (Def->DropBehavior == EWorldItemDropBehavior::DestroyOnDrop)
		{
			FortAthenaInventory::RemoveItem(VictimController, ItemGuid, Item->ItemEntry.Count);
			continue;
		}
		const FVector Dir = FVector::ForwardVector.RotateAngleAxis(BaseYaw + Spawned * 137.5f, FVector::UpVector);
		FVector TossStart, TossFinal, TossDir;
		BuildTossLocations(VictimPawn, Dir, TossStart, TossFinal, TossDir);
		const uint8 SourceFlags = static_cast<uint8>(EFortPickupSourceTypeFlag::Player) |
			static_cast<uint8>(EFortPickupSourceTypeFlag::Tossed);
		const FFortItemEntry Entry = Item->ItemEntry;
		AFortPickupAthena* Pickup = SpawnTossedPickupFromEntry(
			World, Entry, TossStart, TossDir, VictimPawn, SourceFlags, EFortPickupSpawnSource::PlayerElimination);
		if (!Pickup)
		{
			continue;
		}

		const int32 Removed = FortAthenaInventory::RemoveItem(VictimController, ItemGuid, Entry.Count);
		if (Removed <= 0)
		{
			Pickup->Destroy();
			continue;
		}
		Pickup->PrimaryPickupItemEntry.Count = Removed;
		if (Removed < Entry.Count)
		{
			Pickup->PrimaryPickupItemEntry.LoadedAmmo = 0;
		}
		++Spawned;
	}
	UE_LOG(LogTemp, Display, TEXT("LootTransfer: elimination drop pc=%s items=%d dropped=%d"), *GetNameSafe(VictimController), ItemGuids.Num(), Spawned);
	return Spawned;
}
