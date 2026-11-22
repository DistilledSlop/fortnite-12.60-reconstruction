#if WITH_DEV_AUTOMATION_TESTS

#include "ABuildingItemWeaponUpgradeActor.h"
#include "ABuildingWall.h"
#include "AbilitySystemComponent.h"
#include "AFortInventory.h"
#include "AFortPickupAthena.h"
#include "AFortPickupBackpack.h"
#include "AFortPlayerPawnAthena.h"
#include "AFortWeapon.h"
#include "AFortPlayerControllerAthena.h"
#include "AFortPlayerStateAthena.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "EFortDelayedQuickBarAction.h"
#include "EInteractionBeingAttempted.h"
#include "EFortItemType.h"
#include "EFortQuickBars.h"
#include "EFortPickupTossState.h"
#include "EWorldItemDropBehavior.h"
#include "FortAthenaBuildDamage.h"
#include "FortAthenaCombatEquip.h"
#include "FortAthenaInventoryFlow.h"
#include "FortAthenaLootSearch.h"
#include "FortAthenaLootTransfer.h"
#include "FortHarvestBuilding.h"
#include "FortWeaponReload.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "ServerDamageExecution.h"
#include "TInteractionType.h"
#include "UFortAbilitySystemComponent.h"
#include "UFortControllerComponent_Interaction.h"
#include "UFortHealthSet.h"
#include "UFortGameplayAbility.h"
#include "UFortKismetLibrary.h"
#include "UFortResourceItemDefinition.h"
#include "UFortWeaponItemDefinition.h"
#include "UFortWeaponRangedItemDefinition.h"
#include "UFortWorldItem.h"
#include "WeaponUpgradeItemRow.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FServerBlueprintPickupTossTest, "Fortnite.Server.Inventory.BlueprintPickupToss",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)

bool FServerBlueprintPickupTossTest::RunTest(const FString&)
{
	if (!IsRunningDedicatedServer() || !FortAthenaInventory::IsEnabled())
	{
		AddInfo(TEXT("Requires a dedicated server launched with -FortniteOriginalClient."));
		return true;
	}
	UWorld* World = nullptr;
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.World() && Context.World()->GetNetMode() == NM_DedicatedServer)
		{
			World = Context.World();
			break;
		}
	}
	if (!TestNotNull(TEXT("Dedicated server world"), World))
		return false;

	UFortKismetLibrary* Library = GetMutableDefault<UFortKismetLibrary>();
	UFunction* Spawn = Library->FindFunction(TEXT("K2_SpawnPickupInWorld"));
	UFunction* SpawnWithClass = Library->FindFunction(TEXT("K2_SpawnPickupInWorldWithClass"));
	if (!TestNotNull(TEXT("Blueprint pickup spawn function"), Spawn) ||
		!TestNotNull(TEXT("Blueprint pickup spawn with class function"), SpawnWithClass))
		return false;

	UFortResourceItemDefinition* Definition = NewObject<UFortResourceItemDefinition>(World);
	const FVector Position(1200.0f, 2300.0f, 16000.0f);
	const FVector Direction(0.0f, 4.0f, 0.0f);
	// These parameter layouts follow the generated reflected function declarations.
	struct FSpawnParams
	{
		UObject* WorldContextObject; UFortWorldItemDefinition* ItemDefinition;
		int32 NumberToSpawn; FVector Position; FVector Direction; int32 OverrideMaxStackCount;
		bool bToss; bool bRandomRotation; bool bBlockedFromAutoPickup; int32 PickupInstigatorHandle;
		EFortPickupSourceTypeFlag SourceType; EFortPickupSpawnSource Source;
		AFortPlayerController* OptionalOwnerPC; bool bPickupOnlyRelevantToOwner; AFortPickup* ReturnValue;
	};
	struct FSpawnWithClassParams
	{
		UObject* WorldContextObject; UFortWorldItemDefinition* ItemDefinition; UClass* PickupClass;
		int32 NumberToSpawn; FVector Position; FVector Direction; int32 OverrideMaxStackCount;
		bool bToss; bool bRandomRotation; bool bBlockedFromAutoPickup; int32 PickupInstigatorHandle;
		EFortPickupSourceTypeFlag SourceType; EFortPickupSpawnSource Source;
		AFortPlayerController* OptionalOwnerPC; bool bPickupOnlyRelevantToOwner; AFortPickup* ReturnValue;
	};
	FSpawnParams Toss{World, Definition, 1, Position, Direction, 0, true, true, false, 0,
		EFortPickupSourceTypeFlag::Container, EFortPickupSpawnSource::Chest, nullptr, false, nullptr};
	Library->ProcessEvent(Spawn, &Toss);
	FSpawnWithClassParams WithClass{World, Definition, AFortPickupBackpack::StaticClass(), 1, Position, Direction, 0,
		true, true, false, 0, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::PlayerElimination, nullptr, false, nullptr};
	Library->ProcessEvent(SpawnWithClass, &WithClass);
	FSpawnWithClassParams WithoutClass = WithClass;
	WithoutClass.PickupClass = nullptr;
	WithoutClass.ReturnValue = nullptr;
	Library->ProcessEvent(SpawnWithClass, &WithoutClass);
	if (Toss.ReturnValue)
	{
		TestTrue(TEXT("Blueprint pickup defaults to the exact Athena class"), Toss.ReturnValue->GetClass() == AFortPickupAthena::StaticClass());
		TestTrue(TEXT("Blueprint pickup preserves source type"), Toss.ReturnValue->PickupSourceTypeFlags == Toss.SourceType);
		TestTrue(TEXT("Blueprint pickup preserves spawn source"), Toss.ReturnValue->PickupSpawnSource == Toss.Source);
		TestTrue(TEXT("Blueprint pickup preserves random rotation"), Toss.ReturnValue->bRandomRotation);
	}
	if (WithClass.ReturnValue)
	{
		TestTrue(TEXT("Blueprint pickup uses the exact requested backpack class"), WithClass.ReturnValue->GetClass() == AFortPickupBackpack::StaticClass());
		TestTrue(TEXT("Blueprint pickup with class preserves source type"), WithClass.ReturnValue->PickupSourceTypeFlags == WithClass.SourceType);
		TestTrue(TEXT("Blueprint pickup with class preserves spawn source"), WithClass.ReturnValue->PickupSpawnSource == WithClass.Source);
		TestTrue(TEXT("Blueprint pickup with class preserves random rotation"), WithClass.ReturnValue->bRandomRotation);
	}
	if (WithoutClass.ReturnValue)
	{
		TestTrue(TEXT("Blueprint pickup with no class override defaults to the exact Athena class"),
			WithoutClass.ReturnValue->GetClass() == AFortPickupAthena::StaticClass());
	}
	for (AFortPickup* Pickup : {Toss.ReturnValue, WithClass.ReturnValue, WithoutClass.ReturnValue})
	{
		if (TestNotNull(TEXT("Blueprint directional toss spawned a pickup"), Pickup))
		{
			TestTrue(TEXT("Toss begins at the requested position"), Pickup->PickupLocationData.LootInitialPosition.Equals(Position));
			TestTrue(TEXT("Toss travels 180 units along the normalized direction"),
				Pickup->PickupLocationData.LootFinalPosition.Equals(Position + Direction.GetSafeNormal() * 180.0f));
			TestTrue(TEXT("Pickup starts at the toss origin"), Pickup->GetActorLocation().Equals(Position));
			Pickup->Destroy();
		}
	}
	Toss.bToss = false;
	Toss.bRandomRotation = false;
	Toss.ReturnValue = nullptr;
	Library->ProcessEvent(Spawn, &Toss);
	if (TestNotNull(TEXT("Blueprint stationary pickup spawned"), Toss.ReturnValue))
	{
		TestTrue(TEXT("Stationary pickup remains at the requested position"), Toss.ReturnValue->GetActorLocation().Equals(Position));
		TestTrue(TEXT("Stationary pickup does not start a toss"), Toss.ReturnValue->PickupLocationData.TossState != EFortPickupTossState::InProgress);
		TestFalse(TEXT("Stationary pickup preserves disabled random rotation"), Toss.ReturnValue->bRandomRotation);
		Toss.ReturnValue->Destroy();
	}
	Toss.bToss = true;
	Toss.Direction = FVector::ZeroVector;
	Toss.ReturnValue = nullptr;
	Library->ProcessEvent(Spawn, &Toss);
	if (TestNotNull(TEXT("Blueprint zero-direction toss spawned"), Toss.ReturnValue))
	{
		TestTrue(TEXT("Zero-direction toss preserves its destination"), Toss.ReturnValue->PickupLocationData.LootFinalPosition.Equals(Position));
		Toss.ReturnValue->Destroy();
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FServerBlueprintPickupOwnershipTest, "Fortnite.Server.Inventory.BlueprintPickupOwnership",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)

bool FServerBlueprintPickupOwnershipTest::RunTest(const FString&)
{
	if (!IsRunningDedicatedServer() || !FortAthenaInventory::IsEnabled())
	{
		AddInfo(TEXT("Requires a dedicated server launched with -FortniteOriginalClient."));
		return true;
	}
	UWorld* World = nullptr;
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.World() && Context.World()->GetNetMode() == NM_DedicatedServer)
		{
			World = Context.World();
			break;
		}
	}
	if (!TestNotNull(TEXT("Dedicated server world"), World))
		return false;

	AFortPlayerControllerAthena* Owner = World->SpawnActor<AFortPlayerControllerAthena>();
	AFortPlayerControllerAthena* Other = World->SpawnActor<AFortPlayerControllerAthena>();
	AFortInventory* OwnerInventory = World->SpawnActor<AFortInventory>();
	AFortInventory* OtherInventory = World->SpawnActor<AFortInventory>();
	const FVector Position(1200.0f, 2300.0f, 16000.0f);
	AFortPlayerPawnAthena* OwnerPawn = World->SpawnActor<AFortPlayerPawnAthena>(Position, FRotator::ZeroRotator);
	AFortPlayerPawnAthena* OtherPawn = World->SpawnActor<AFortPlayerPawnAthena>(Position + FVector(150, 0, 0), FRotator::ZeroRotator);
	ON_SCOPE_EXIT
	{
		if (Owner) { Owner->WorldInventory = nullptr; Owner->Destroy(); }
		if (Other) { Other->WorldInventory = nullptr; Other->Destroy(); }
		if (OwnerPawn) OwnerPawn->Destroy();
		if (OtherPawn) OtherPawn->Destroy();
		if (OwnerInventory) OwnerInventory->Destroy();
		if (OtherInventory) OtherInventory->Destroy();
	};
	if (!TestNotNull(TEXT("Owner controller"), Owner) || !TestNotNull(TEXT("Other controller"), Other) ||
		!TestNotNull(TEXT("Owner inventory"), OwnerInventory) || !TestNotNull(TEXT("Other inventory"), OtherInventory) ||
		!TestNotNull(TEXT("Owner pawn"), OwnerPawn) || !TestNotNull(TEXT("Other pawn"), OtherPawn))
		return false;
	Owner->WorldInventory = OwnerInventory;
	Other->WorldInventory = OtherInventory;
	Owner->Possess(OwnerPawn);
	Other->Possess(OtherPawn);
	if (!TestTrue(TEXT("Owner controls its pawn"), OwnerPawn->GetController() == Owner) ||
		!TestTrue(TEXT("Other player controls its pawn"), OtherPawn->GetController() == Other))
		return false;

	UFortKismetLibrary* Library = GetMutableDefault<UFortKismetLibrary>();
	UFunction* Spawn = Library->FindFunction(TEXT("K2_SpawnPickupInWorld"));
	if (!TestNotNull(TEXT("Blueprint pickup spawn function"), Spawn))
		return false;
	UFortResourceItemDefinition* Definition = NewObject<UFortResourceItemDefinition>(World);
	Definition->ItemType = EFortItemType::WorldResource;
	Definition->MaxStackSize = FScalableFloat(10.0f);
	Definition->bAllowMultipleStacks = true;
	// This layout follows the generated reflected function declaration.
	struct FSpawnParams
	{
		UObject* WorldContextObject; UFortWorldItemDefinition* ItemDefinition;
		int32 NumberToSpawn; FVector Position; FVector Direction; int32 OverrideMaxStackCount;
		bool bToss; bool bRandomRotation; bool bBlockedFromAutoPickup; int32 PickupInstigatorHandle;
		EFortPickupSourceTypeFlag SourceType; EFortPickupSpawnSource Source;
		AFortPlayerController* OptionalOwnerPC; bool bPickupOnlyRelevantToOwner; AFortPickup* ReturnValue;
	};
	FSpawnParams Params{World, Definition, 1, Position, FVector::ZeroVector, 0, false, false, false, 0,
		EFortPickupSourceTypeFlag::Other, EFortPickupSpawnSource::Unset, nullptr, true, nullptr};
	Library->ProcessEvent(Spawn, &Params);
	TestNull(TEXT("Private pickup requires an owner"), Params.ReturnValue);
	if (Params.ReturnValue) Params.ReturnValue->Destroy();
	Params.OptionalOwnerPC = Owner;
	Params.ReturnValue = nullptr;
	Library->ProcessEvent(Spawn, &Params);
	AFortPickup* PrivatePickup = Params.ReturnValue;
	if (!TestNotNull(TEXT("Private pickup spawned"), PrivatePickup))
		return false;
	ON_SCOPE_EXIT { PrivatePickup->Destroy(); };
	TestTrue(TEXT("Private pickup has the requested owner"), PrivatePickup->GetOwner() == Owner);
	TestTrue(TEXT("Private pickup is only relevant to its owner"), PrivatePickup->bOnlyRelevantToOwner);
	OtherPawn->ServerHandlePickup(PrivatePickup, 0.25f, FVector::ZeroVector, false);
	TestEqual(TEXT("Other player receives no items from the private pickup"), OtherInventory->Inventory.ReplicatedEntries.Num(), 0);
	TestFalse(TEXT("Rejected private pickup remains available"), PrivatePickup->bPickedUp);
	TestEqual(TEXT("Rejected private pickup retains its count"), PrivatePickup->PrimaryPickupItemEntry.Count, 1);
	OwnerPawn->ServerHandlePickup(PrivatePickup, 0.25f, FVector::ZeroVector, false);
	TestTrue(TEXT("Owner can collect the private pickup"), PrivatePickup->bPickedUp);
	if (TestEqual(TEXT("Owner receives one stack"), OwnerInventory->Inventory.ReplicatedEntries.Num(), 1))
	{
		TestTrue(TEXT("Owner receives the requested item"), OwnerInventory->Inventory.ReplicatedEntries[0].ItemDefinition == Definition);
		TestEqual(TEXT("Owner receives one unit"), OwnerInventory->Inventory.ReplicatedEntries[0].Count, 1);
	}
	Params.bPickupOnlyRelevantToOwner = false;
	Params.ReturnValue = nullptr;
	Library->ProcessEvent(Spawn, &Params);
	if (TestNotNull(TEXT("Public pickup spawned"), Params.ReturnValue))
	{
		AFortPickup* PublicPickup = Params.ReturnValue;
		ON_SCOPE_EXIT { PublicPickup->Destroy(); };
		TestTrue(TEXT("Public pickup retains the requested owner"), PublicPickup->GetOwner() == Owner);
		TestFalse(TEXT("Public pickup is relevant to other players"), PublicPickup->bOnlyRelevantToOwner);
		OtherPawn->ServerHandlePickup(PublicPickup, 0.25f, FVector::ZeroVector, false);
		TestTrue(TEXT("Other player can collect the public pickup"), PublicPickup->bPickedUp);
		TestEqual(TEXT("Other player receives the public pickup"), OtherInventory->Inventory.ReplicatedEntries.Num(), 1);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FServerInventoryStacksTest, "Fortnite.Server.Inventory.StackAccounting",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)

bool FServerInventoryStacksTest::RunTest(const FString&)
{
	if (!IsRunningDedicatedServer() || !FortAthenaInventory::IsEnabled())
	{
		AddInfo(TEXT("Requires a dedicated server launched with -FortniteOriginalClient."));
		return true;
	}

	UWorld* World = nullptr;
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.World() && Context.World()->GetNetMode() == NM_DedicatedServer)
		{
			World = Context.World();
			break;
		}
	}
	if (!TestNotNull(TEXT("Dedicated server world"), World))
	{
		return false;
	}

	AFortPlayerControllerAthena* Controller = World->SpawnActor<AFortPlayerControllerAthena>();
	AFortInventory* Inventory = World->SpawnActor<AFortInventory>();
	ON_SCOPE_EXIT
	{
		if (Controller)
		{
			Controller->WorldInventory = nullptr;
			Controller->Destroy();
		}
		if (Inventory)
		{
			Inventory->Destroy();
		}
	};
	if (!TestNotNull(TEXT("Inventory controller"), Controller) || !TestNotNull(TEXT("World inventory"), Inventory))
	{
		return false;
	}
	Controller->WorldInventory = Inventory;

	UFortResourceItemDefinition* Definition = NewObject<UFortResourceItemDefinition>(Inventory);
	Definition->ItemType = EFortItemType::WorldResource;
	Definition->MaxStackSize = FScalableFloat(10.0f);
	Definition->bAllowMultipleStacks = true;

	int32 Accepted = -1;
	TestNull(TEXT("A zero-quantity grant is rejected"),
		FortAthenaInventory::GiveItem(Controller, Definition, 0, 1, 0, &Accepted));
	TestEqual(TEXT("A rejected zero-quantity grant accepts nothing"), Accepted, 0);
	TestEqual(TEXT("A rejected grant leaves no item instance"), Inventory->Inventory.ItemInstances.Num(), 0);
	TestEqual(TEXT("A rejected grant leaves no replicated entry"), Inventory->Inventory.ReplicatedEntries.Num(), 0);

	UFortWorldItem* First = FortAthenaInventory::GiveItem(Controller, Definition, 12, 1, 0, &Accepted);
	if (!TestNotNull(TEXT("First stack"), First))
	{
		return false;
	}
	TestEqual(TEXT("Oversized grant accepts one capped stack"), Accepted, 10);
	TestEqual(TEXT("First stack count"), First->ItemEntry.Count, 10);

	UFortWorldItem* Second = FortAthenaInventory::GiveItem(Controller, Definition, 6);
	if (!TestNotNull(TEXT("Second stack"), Second) || !TestTrue(TEXT("Second stack has its own instance"), Second != First))
	{
		return false;
	}
	const FGuid FirstGuid = First->NativeGetItemGuid();
	const FGuid SecondGuid = Second->NativeGetItemGuid();
	UFortWorldItem* ToppedUp = FortAthenaInventory::GiveItem(Controller, Definition, 3, 1, 0, &Accepted);
	TestTrue(TEXT("Grant fills the partial stack past the full first stack"), ToppedUp == Second);
	TestEqual(TEXT("Top-up accepted quantity"), Accepted, 3);
	TestEqual(TEXT("Top-up pads the second stack"), Second->ItemEntry.Count, 9);
	TestEqual(TEXT("Top-up leaves the stack count unchanged"), Inventory->Inventory.ItemInstances.Num(), 2);

	TestEqual(TEXT("Debit spans both stacks"), FortAthenaInventory::ConsumeAmmo(Controller, Definition, 12), 12);
	TestNull(TEXT("Depleted stack is removed from the GUID index"), Inventory->NativeFindItemByGuid(FirstGuid));
	TestEqual(TEXT("Remaining stack quantity"), Second->ItemEntry.Count, 7);
	if (TestEqual(TEXT("Exactly one replicated stack remains"), Inventory->Inventory.ReplicatedEntries.Num(), 1))
	{
		const FFortItemEntry& Entry = Inventory->Inventory.ReplicatedEntries[0];
		TestTrue(TEXT("Replicated stack retains the surviving GUID"), Entry.ItemGuid == SecondGuid);
		TestEqual(TEXT("Replicated quantity matches the surviving stack"), Entry.Count, 7);
	}
	UFortWorldItem* MergeSource = FortAthenaInventory::GiveItem(Controller, Definition, 7);
	if (!TestNotNull(TEXT("Overflow creates a source stack for merging"), MergeSource) ||
		!TestTrue(TEXT("Merge source is a separate stack"), MergeSource != Second))
	{
		return false;
	}
	const FGuid MergeSourceGuid = MergeSource->NativeGetItemGuid();
	TestEqual(TEXT("Grant fills the original stack before creating the merge source"), Second->ItemEntry.Count, 10);
	TestEqual(TEXT("Overflow source keeps four units"), MergeSource->ItemEntry.Count, 4);
	TestEqual(TEXT("A full destination rejects a merge"), FortAthenaInventory::CombineStacks(Controller, SecondGuid, MergeSourceGuid), 0);
	TestEqual(TEXT("Free two units of stack capacity"), FortAthenaInventory::RemoveItem(Controller, SecondGuid, 2), 2);
	TestEqual(TEXT("Merge moves only the available capacity"), FortAthenaInventory::CombineStacks(Controller, SecondGuid, MergeSourceGuid), 2);
	TestEqual(TEXT("Merge fills the destination"), Second->ItemEntry.Count, 10);
	TestEqual(TEXT("Merge preserves the excess in the source"), MergeSource->ItemEntry.Count, 2);
	TestEqual(TEXT("Free room for the remaining source"), FortAthenaInventory::RemoveItem(Controller, SecondGuid, 5), 5);
	TestEqual(TEXT("Merge moves the remaining source"), FortAthenaInventory::CombineStacks(Controller, SecondGuid, MergeSourceGuid), 2);
	TestNull(TEXT("Merge removes the drained source GUID"), Inventory->NativeFindItemByGuid(MergeSourceGuid));
	if (TestEqual(TEXT("Only the destination remains replicated after the merge"), Inventory->Inventory.ReplicatedEntries.Num(), 1))
	{
		const FFortItemEntry& Entry = Inventory->Inventory.ReplicatedEntries[0];
		TestTrue(TEXT("Merged entry retains its destination GUID"), Entry.ItemGuid == SecondGuid);
		TestEqual(TEXT("Replicated destination has the merged count"), Entry.Count, 7);
	}

	AFortPlayerPawnAthena* Pawn = World->SpawnActor<AFortPlayerPawnAthena>(FVector(0, 0, 16000), FRotator::ZeroRotator);
	if (!TestNotNull(TEXT("Inventory transfer pawn"), Pawn))
	{
		return false;
	}
	ON_SCOPE_EXIT { Pawn->Destroy(); };
	Controller->Possess(Pawn);
	{
		for (FQuickBarData& Bar : Controller->QuickBarDefinitions)
		{
			Bar.QuickbarSlots.Reset();
		}
		TArray<FGuid> LayoutItems;
		ON_SCOPE_EXIT
		{
			for (const FGuid& Guid : LayoutItems)
			{
				FortAthenaInventory::RemoveItem(Controller, Guid, 1);
			}
		};
		auto GiveSlottedItem = [&](EFortItemType Type, EFortQuickBars Bar, int32 Slot)
		{
			UFortWeaponRangedItemDefinition* ItemDefinition = NewObject<UFortWeaponRangedItemDefinition>(Inventory);
			ItemDefinition->ItemType = Type;
			ItemDefinition->MaxStackSize = FScalableFloat(1.0f);
			UFortWorldItem* Item = FortAthenaInventory::GiveItem(Controller, ItemDefinition, 1);
			if (TestNotNull(TEXT("Quickbar item grant succeeds"), Item))
			{
				const FGuid Guid = Item->NativeGetItemGuid();
				LayoutItems.Add(Guid);
				TestTrue(TEXT("Quickbar add publishes the expected bar, slot and item GUID"),
					Controller->DelayedQuickBarActions.Items.ContainsByPredicate([&](const FDelayedQuickBarAction& Action)
					{
						return Action.Action == EFortDelayedQuickBarAction::Add && Action.ItemGuid == Guid &&
							Action.QuickBarType == Bar && Action.QuickBarSlot == Slot;
					}));
			}
		};
		GiveSlottedItem(EFortItemType::WeaponHarvest, EFortQuickBars::Primary, 0);
		GiveSlottedItem(EFortItemType::Trap, EFortQuickBars::Secondary, 4);
		for (int32 Slot = 1; Slot <= 5; ++Slot)
		{
			GiveSlottedItem(EFortItemType::WeaponRanged, EFortQuickBars::Primary, Slot);
		}
		TestFalse(TEXT("Materials remain in inventory without a quickbar slot"),
			Controller->DelayedQuickBarActions.Items.ContainsByPredicate([&](const FDelayedQuickBarAction& Action)
			{
				return Action.Action == EFortDelayedQuickBarAction::Add && Action.ItemGuid == SecondGuid;
			}));

		UFortWeaponRangedItemDefinition* ExtraWeapon = NewObject<UFortWeaponRangedItemDefinition>(Inventory);
		ExtraWeapon->ItemType = EFortItemType::WeaponRanged;
		ExtraWeapon->MaxStackSize = FScalableFloat(1.0f);
		FFortItemEntry ExtraEntry{};
		ExtraEntry.ItemDefinition = ExtraWeapon;
		ExtraEntry.Count = 1;
		AFortPickupAthena* ExtraPickup = FortAthenaLootTransfer::SpawnTossedPickupFromEntry(
			World, ExtraEntry, Pawn->GetActorLocation(), FVector::ForwardVector, nullptr, 0, EFortPickupSpawnSource::Unset);
		if (TestNotNull(TEXT("Sixth weapon world pickup"), ExtraPickup))
		{
			TestFalse(TEXT("A sixth weapon cannot be picked up with all loadout slots occupied"),
				FortAthenaLootTransfer::HandlePickup(Pawn, ExtraPickup, 0.25f, FVector::ZeroVector, false));
			TestEqual(TEXT("Rejected weapon stays on the ground"), ExtraPickup->PrimaryPickupItemEntry.Count, 1);
			TestNull(TEXT("Rejected weapon does not enter inventory"), Inventory->NativeFindWorldItemByDefinition(ExtraWeapon, true));
			ExtraPickup->Destroy();
		}
	}
	Definition->bCanBeDropped = true;
	Definition->bAllowMultipleStacks = false;

	FFortItemEntry WorldEntry{};
	WorldEntry.ItemDefinition = Definition;
	WorldEntry.Count = 5;
	AFortPickupAthena* Pickup = FortAthenaLootTransfer::SpawnTossedPickupFromEntry(
		World, WorldEntry, Pawn->GetActorLocation(), FVector::ForwardVector, nullptr, 0, EFortPickupSpawnSource::Unset);
	if (!TestNotNull(TEXT("World pickup"), Pickup))
	{
		return false;
	}
	ON_SCOPE_EXIT { Pickup->Destroy(); };

	TestTrue(TEXT("Pickup fills the existing stack"), FortAthenaLootTransfer::HandlePickup(Pawn, Pickup, 0.25f, FVector::ZeroVector, false));
	TestEqual(TEXT("Inventory reaches its cap"), Second->ItemEntry.Count, 10);
	TestEqual(TEXT("Two units stay on the ground"), Pickup->PrimaryPickupItemEntry.Count, 2);
	TestFalse(TEXT("A retry at the stack cap accepts nothing"),
		FortAthenaLootTransfer::HandlePickup(Pawn, Pickup, 0.25f, FVector::ZeroVector, false));
	TestEqual(TEXT("Rejected pickup preserves the inventory total"), Second->ItemEntry.Count, 10);
	TestEqual(TEXT("Rejected pickup preserves the ground remainder"), Pickup->PrimaryPickupItemEntry.Count, 2);

	Controller->ServerAttemptInventoryDrop(SecondGuid, 0, false);
	TestEqual(TEXT("A zero-count drop preserves the stack"), Second->ItemEntry.Count, 10);
	Controller->ServerAttemptInventoryDrop(SecondGuid, 4, false);
	TestEqual(TEXT("Six units stay in inventory"), Second->ItemEntry.Count, 6);
	AFortPickupAthena* Dropped = nullptr;
	for (TActorIterator<AFortPickupAthena> It(World); It; ++It)
	{
		if (It->PawnWhoDroppedPickup == Pawn && It->PrimaryPickupItemEntry.ItemDefinition == Definition)
		{
			Dropped = *It;
			break;
		}
	}
	if (TestNotNull(TEXT("Split stack produced a world pickup"), Dropped))
	{
		TestEqual(TEXT("Split drop contains exactly four units"), Dropped->PrimaryPickupItemEntry.Count, 4);
	}
	ON_SCOPE_EXIT { if (Dropped) { Dropped->Destroy(); } };

	TestTrue(TEXT("Remaining ground units can be picked up after the split"), FortAthenaLootTransfer::HandlePickup(Pawn, Pickup, 0.25f, FVector::ZeroVector, false));
	TestEqual(TEXT("Inventory, split drop and consumed ground stack account for all units"), Second->ItemEntry.Count, 8);
	TestTrue(TEXT("Exhausted ground pickup is reserved for flight"), Pickup->bPickedUp);
	TestFalse(TEXT("A claimed pickup cannot grant items again"),
		FortAthenaLootTransfer::HandlePickup(Pawn, Pickup, 0.25f, FVector::ZeroVector, false));
	AFortPlayerControllerAthena* Contender = World->SpawnActor<AFortPlayerControllerAthena>();
	AFortInventory* ContenderInventory = World->SpawnActor<AFortInventory>();
	AFortPlayerPawnAthena* ContenderPawn = World->SpawnActor<AFortPlayerPawnAthena>(
		Pawn->GetActorLocation() + FVector(150, 0, 0), FRotator::ZeroRotator);
	ON_SCOPE_EXIT
	{
		if (Contender)
		{
			Contender->WorldInventory = nullptr;
			Contender->Destroy();
		}
		if (ContenderPawn)
		{
			ContenderPawn->Destroy();
		}
		if (ContenderInventory)
		{
			ContenderInventory->Destroy();
		}
	};
	if (!TestNotNull(TEXT("Pickup contender controller"), Contender) ||
		!TestNotNull(TEXT("Pickup contender inventory"), ContenderInventory) ||
		!TestNotNull(TEXT("Pickup contender pawn"), ContenderPawn))
	{
		return false;
	}
	Contender->Possess(ContenderPawn);
	Contender->WorldInventory = ContenderInventory;
	if (!TestTrue(TEXT("Contender controls its pawn"), ContenderPawn->GetController() == Contender))
	{
		return false;
	}
	ContenderPawn->ServerHandlePickup(Pickup, 0.25f, FVector::ZeroVector, false);
	TestEqual(TEXT("The losing contender receives no replicated items"), ContenderInventory->Inventory.ReplicatedEntries.Num(), 0);
	if (!Dropped)
	{
		return false;
	}
	ContenderPawn->ServerHandlePickup(Dropped, 0.25f, FVector::ZeroVector, false);
	TestTrue(TEXT("The second player claims the split drop"), Dropped->bPickedUp);
	ContenderPawn->ServerHandlePickup(Dropped, 0.25f, FVector::ZeroVector, false);
	if (TestEqual(TEXT("The second player receives one replicated stack"), ContenderInventory->Inventory.ReplicatedEntries.Num(), 1))
	{
		const FFortItemEntry& Entry = ContenderInventory->Inventory.ReplicatedEntries[0];
		TestTrue(TEXT("The received stack retains its item definition"), Entry.ItemDefinition == Definition);
		TestEqual(TEXT("The second player receives exactly the four dropped units"), Entry.Count, 4);
	}
	Dropped->Destroy();
	TestEqual(TEXT("Claim retry preserves the inventory total"), Second->ItemEntry.Count, 8);
	if (TestEqual(TEXT("Exactly one replicated stack survives the transfers"), Inventory->Inventory.ReplicatedEntries.Num(), 1))
	{
		const FFortItemEntry& Entry = Inventory->Inventory.ReplicatedEntries[0];
		TestTrue(TEXT("Transfers keep the surviving stack GUID"), Entry.ItemGuid == SecondGuid);
		TestEqual(TEXT("Transfers reconcile the replicated stack count"), Entry.Count, 8);
	}

	if (!TestTrue(TEXT("Discarding a whole stack succeeds"),
		FortAthenaLootTransfer::AttemptInventoryDrop(Controller, SecondGuid, 8, true)))
	{
		return false;
	}
	TestNull(TEXT("Discarded stack is removed from the GUID index"), Inventory->NativeFindItemByGuid(SecondGuid));
	TestEqual(TEXT("Discard clears the item instance"), Inventory->Inventory.ItemInstances.Num(), 0);
	TestEqual(TEXT("Discard clears the replicated stack"), Inventory->Inventory.ReplicatedEntries.Num(), 0);
	TestEqual(TEXT("Discarded units cannot be spent again"), FortAthenaInventory::ConsumeAmmo(Controller, Definition, 1), 0);
	for (TActorIterator<AFortPickupAthena> It(World); It; ++It)
	{
		if (It->PawnWhoDroppedPickup == Pawn && It->PrimaryPickupItemEntry.ItemDefinition == Definition)
		{
			AddError(TEXT("Discard spawned a collectible pickup"));
			break;
		}
	}

	UFortWeaponRangedItemDefinition* WeaponDefinition = NewObject<UFortWeaponRangedItemDefinition>(Inventory);
	WeaponDefinition->bCanBeDropped = true;
	WeaponDefinition->DropBehavior = EWorldItemDropBehavior::DropAsPickup;
	WeaponDefinition->MaxStackSize = FScalableFloat(1.0f);
	FFortItemEntry LoadedBundle{};
	LoadedBundle.ItemDefinition = WeaponDefinition;
	LoadedBundle.Count = 2;
	LoadedBundle.LoadedAmmo = 5;
	LoadedBundle.Durability = 42.0f;
	AFortPickupAthena* LoadedPickup = FortAthenaLootTransfer::SpawnTossedPickupFromEntry(
		World, LoadedBundle, Pawn->GetActorLocation(), FVector::ForwardVector, nullptr, 0, EFortPickupSpawnSource::Unset);
	if (!TestNotNull(TEXT("Loaded pickup bundle"), LoadedPickup))
	{
		return false;
	}
	ON_SCOPE_EXIT { LoadedPickup->Destroy(); };
	if (!TestTrue(TEXT("One weapon can be collected from the bundle"),
		FortAthenaLootTransfer::HandlePickup(Pawn, LoadedPickup, 0.25f, FVector::ZeroVector, false)) ||
		!TestEqual(TEXT("One loaded weapon enters inventory"), Inventory->Inventory.ReplicatedEntries.Num(), 1))
	{
		return false;
	}
	const FFortItemEntry& LoadedWeapon = Inventory->Inventory.ReplicatedEntries[0];
	TestEqual(TEXT("First grant keeps the original magazine"), LoadedWeapon.LoadedAmmo, 5);
	TestEqual(TEXT("First grant keeps the original durability"), LoadedWeapon.Durability, 42.0f);
	TestEqual(TEXT("The remaining weapon keeps its durability"), LoadedPickup->PrimaryPickupItemEntry.Durability, 42.0f);
	TestEqual(TEXT("One weapon remains in the world"), LoadedPickup->PrimaryPickupItemEntry.Count, 1);
	TestEqual(TEXT("Remainder does not duplicate the magazine"), LoadedPickup->PrimaryPickupItemEntry.LoadedAmmo, 0);
	const FGuid LoadedGuid = LoadedWeapon.ItemGuid;
	TestEqual(TEXT("Remove the weapon from the partial pickup fixture"), FortAthenaInventory::RemoveItem(Controller, LoadedGuid, 1), 1);

	WeaponDefinition->MaxStackSize = FScalableFloat(2.0f);
	WeaponDefinition->bAllowMultipleStacks = true;
	UFortWorldItem* EmptyStack = FortAthenaInventory::GiveItem(Controller, WeaponDefinition, 1, 1, 0);
	UFortWorldItem* LoadedStack = FortAthenaInventory::GiveItem(Controller, WeaponDefinition, 1, 1, 5);
	if (!TestNotNull(TEXT("Empty weapon stack"), EmptyStack) ||
		!TestNotNull(TEXT("Loaded weapon stack"), LoadedStack) ||
		!TestTrue(TEXT("A loaded grant creates a separate entry"), EmptyStack != LoadedStack))
	{
		return false;
	}
	UFortWorldItem* HigherLevelStack = FortAthenaInventory::GiveItem(Controller, WeaponDefinition, 1, 3);
	if (!TestNotNull(TEXT("Weapon grant at a different level"), HigherLevelStack))
	{
		return false;
	}
	const FGuid HigherLevelGuid = HigherLevelStack->NativeGetItemGuid();
	TestTrue(TEXT("A different-level grant creates a separate stack"), HigherLevelStack != EmptyStack);
	TestEqual(TEXT("A different-level grant keeps its level"), HigherLevelStack->ItemEntry.Level, 3);
	TestEqual(TEXT("An explicit merge cannot consume a different-level stack"),
		FortAthenaInventory::CombineStacks(Controller, EmptyStack->NativeGetItemGuid(), HigherLevelGuid), 0);
	TestEqual(TEXT("The original stack stays at one unit"), EmptyStack->ItemEntry.Count, 1);
	TestEqual(TEXT("The higher-level stack stays at one unit"), HigherLevelStack->ItemEntry.Count, 1);
	TestEqual(TEXT("Remove the higher-level test stack"), FortAthenaInventory::RemoveItem(Controller, HigherLevelGuid, 1), 1);
	const FGuid EmptyStackGuid = EmptyStack->NativeGetItemGuid();
	const FGuid LoadedStackGuid = LoadedStack->NativeGetItemGuid();
	TestEqual(TEXT("Loaded grant retains its magazine"), LoadedStack->ItemEntry.LoadedAmmo, 5);
	TestEqual(TEXT("Merging a loaded source is rejected"), FortAthenaInventory::CombineStacks(Controller, EmptyStackGuid, LoadedStackGuid), 0);
	TestEqual(TEXT("An empty source can merge into a loaded destination"), FortAthenaInventory::CombineStacks(Controller, LoadedStackGuid, EmptyStackGuid), 1);
	TestEqual(TEXT("The merge preserves the destination magazine"), LoadedStack->ItemEntry.LoadedAmmo, 5);
	TestEqual(TEXT("Remove the merged fixture stack"), FortAthenaInventory::RemoveItem(Controller, LoadedStackGuid, 2), 2);
	WeaponDefinition->MaxStackSize = FScalableFloat(1.0f);

	UFortWorldItem* WeaponItem = FortAthenaInventory::GiveItem(Controller, WeaponDefinition, 1, 1, 5);
	if (!TestNotNull(TEXT("Weapon inventory item"), WeaponItem))
	{
		return false;
	}
	const FGuid WeaponGuid = WeaponItem->NativeGetItemGuid();
	FortAthenaInventory::SetLoadedAmmo(Controller, WeaponGuid, 4);
	TestEqual(TEXT("Magazine spend updates the weapon item"), WeaponItem->ItemEntry.LoadedAmmo, 4);
	if (TestEqual(TEXT("The weapon is the only replicated item"), Inventory->Inventory.ReplicatedEntries.Num(), 1))
	{
		const FFortItemEntry& Entry = Inventory->Inventory.ReplicatedEntries[0];
		TestTrue(TEXT("Magazine spend keeps the weapon GUID"), Entry.ItemGuid == WeaponGuid);
		TestEqual(TEXT("Magazine spend updates the replicated entry"), Entry.LoadedAmmo, 4);
	}
	FortAthenaInventory::SetLoadedAmmo(Controller, WeaponGuid, 0);
	if (TestEqual(TEXT("The empty weapon is still replicated"), Inventory->Inventory.ReplicatedEntries.Num(), 1))
	{
		TestEqual(TEXT("An empty magazine replicates as zero"), Inventory->Inventory.ReplicatedEntries[0].LoadedAmmo, 0);
	}
	if (!TestTrue(TEXT("Dropping the empty weapon succeeds"),
		FortAthenaLootTransfer::AttemptInventoryDrop(Controller, WeaponGuid, 1, false)))
	{
		return false;
	}
	TestNull(TEXT("The dropped weapon leaves inventory"), Inventory->NativeFindItemByGuid(WeaponGuid));
	AFortPickupAthena* DroppedWeapon = nullptr;
	for (TActorIterator<AFortPickupAthena> It(World); It; ++It)
	{
		if (It->PawnWhoDroppedPickup == Pawn && It->PrimaryPickupItemEntry.ItemDefinition == WeaponDefinition)
		{
			DroppedWeapon = *It;
			break;
		}
	}
	if (!TestNotNull(TEXT("Dropping the weapon produces a pickup"), DroppedWeapon))
	{
		return false;
	}
	ON_SCOPE_EXIT { DroppedWeapon->Destroy(); };
	TestEqual(TEXT("The ground weapon retains its empty magazine"), DroppedWeapon->PrimaryPickupItemEntry.LoadedAmmo, 0);
	if (!TestTrue(TEXT("The ground weapon can be picked up again"),
		FortAthenaLootTransfer::HandlePickup(Pawn, DroppedWeapon, 0.25f, FVector::ZeroVector, false)) ||
		!TestEqual(TEXT("Exactly one weapon is back in inventory"), Inventory->Inventory.ReplicatedEntries.Num(), 1))
	{
		return false;
	}
	TestEqual(TEXT("Pickup does not refill the magazine"), Inventory->Inventory.ReplicatedEntries[0].LoadedAmmo, 0);
	const FGuid RecoveredGuid = Inventory->Inventory.ReplicatedEntries[0].ItemGuid;

	Definition->bCanBeDropped = false;
	Definition->DropBehavior = EWorldItemDropBehavior::DropAsPickup;
	UFortWorldItem* ProtectedItem = FortAthenaInventory::GiveItem(Controller, Definition, 3);
	if (!TestNotNull(TEXT("Protected inventory item"), ProtectedItem))
	{
		return false;
	}
	const FGuid ProtectedGuid = ProtectedItem->NativeGetItemGuid();
	UFortResourceItemDefinition* DiscardDefinition = NewObject<UFortResourceItemDefinition>(Inventory);
	DiscardDefinition->MaxStackSize = FScalableFloat(10.0f);
	DiscardDefinition->bCanBeDropped = true;
	DiscardDefinition->DropBehavior = EWorldItemDropBehavior::DestroyOnDrop;
	UFortWorldItem* DiscardItem = FortAthenaInventory::GiveItem(Controller, DiscardDefinition, 2);
	if (!TestNotNull(TEXT("Destroy-on-drop inventory item"), DiscardItem))
	{
		return false;
	}
	const FGuid DiscardGuid = DiscardItem->NativeGetItemGuid();
	TestEqual(TEXT("Elimination drops only the eligible weapon"), FortAthenaLootTransfer::DropAllOnDeath(Controller, Pawn), 1);
	TestNull(TEXT("Elimination removes the dropped weapon from inventory"), Inventory->NativeFindItemByGuid(RecoveredGuid));
	TestNull(TEXT("Elimination consumes the destroy-on-drop stack"), Inventory->NativeFindItemByGuid(DiscardGuid));
	if (TestEqual(TEXT("Only the protected stack stays in replicated inventory"), Inventory->Inventory.ReplicatedEntries.Num(), 1))
	{
		const FFortItemEntry& Entry = Inventory->Inventory.ReplicatedEntries[0];
		TestTrue(TEXT("The protected stack retains its GUID"), Entry.ItemGuid == ProtectedGuid);
		TestEqual(TEXT("Elimination preserves the protected stack quantity"), Entry.Count, 3);
	}
	int32 WeaponPickups = 0;
	for (TActorIterator<AFortPickupAthena> It(World); It; ++It)
	{
		if (It->PawnWhoDroppedPickup != Pawn || *It == DroppedWeapon)
		{
			continue;
		}
		TestTrue(TEXT("Elimination loot contains only the eligible weapon"), It->PrimaryPickupItemEntry.ItemDefinition == WeaponDefinition);
		TestEqual(TEXT("Elimination transfers exactly one weapon"), It->PrimaryPickupItemEntry.Count, 1);
		++WeaponPickups;
		It->Destroy();
	}
	TestEqual(TEXT("Elimination creates one new pickup"), WeaponPickups, 1);

	ABuildingWall* Wall = World->SpawnActor<ABuildingWall>(
		Pawn->GetActorLocation() + FVector(150, 0, 0), FRotator::ZeroRotator);
	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(Controller->PlayerState);
	ON_SCOPE_EXIT { if (Wall) { Wall->Destroy(); } };
	if (!TestNotNull(TEXT("Repair test wall"), Wall) || !TestNotNull(TEXT("Repairing player state"), PlayerState))
	{
		return false;
	}
	Controller->bCraftFree = false;
	Wall->bPlayerPlaced = true;
	Wall->SetTeam(PlayerState->TeamIndex);
	Wall->ResourceType = EFortResourceType::Wood;
	Wall->MaxHealthInitializationValue = 100.0f;
	FortHarvest::InitializeBuildingHealth(Wall);
	FortHarvest::ForceBuildingHealth(Wall, 50.0f);
	const int32 RepairCost = FortAthenaBuildDamage::ComputeRepairCost(Wall);
	if (!TestTrue(TEXT("The damaged wall costs enough to test split payment"), RepairCost > 1))
	{
		return false;
	}
	Definition->ResourceType = EFortResourceType::Stone;
	TestFalse(TEXT("Repair rejects the wrong material"), FortAthenaBuildDamage::RepairBuildingActor(Controller, Wall));
	TestEqual(TEXT("Rejected repair leaves the wall damaged"), FortHarvest::GetBuildingHealth(Wall), 50.0f);
	TestEqual(TEXT("Rejected repair preserves the unrelated stack"), ProtectedItem->ItemEntry.Count, 3);

	UFortResourceItemDefinition* RepairMaterial = NewObject<UFortResourceItemDefinition>(Inventory);
	RepairMaterial->ResourceType = EFortResourceType::Wood;
	RepairMaterial->MaxStackSize = FScalableFloat(static_cast<float>(RepairCost + 1));
	UFortWorldItem* FirstRepairStack = FortAthenaInventory::GiveItem(Controller, RepairMaterial, RepairCost - 1);
	if (!TestNotNull(TEXT("First repair material stack"), FirstRepairStack))
	{
		return false;
	}
	const FGuid FirstRepairGuid = FirstRepairStack->NativeGetItemGuid();
	TestFalse(TEXT("Repair rejects a matching but insufficient material balance"),
		FortAthenaBuildDamage::RepairBuildingActor(Controller, Wall));
	TestEqual(TEXT("Rejected repair preserves the partial payment"), FirstRepairStack->ItemEntry.Count, RepairCost - 1);
	TestEqual(TEXT("Rejected repair leaves building health unchanged"), FortHarvest::GetBuildingHealth(Wall), 50.0f);

	UFortResourceItemDefinition* OtherWood = NewObject<UFortResourceItemDefinition>(Inventory);
	OtherWood->ResourceType = EFortResourceType::Wood;
	OtherWood->MaxStackSize = FScalableFloat(2.0f);
	UFortWorldItem* SecondRepairStack = FortAthenaInventory::GiveItem(Controller, OtherWood, 2);
	if (!TestNotNull(TEXT("Second repair material stack"), SecondRepairStack))
	{
		return false;
	}
	TestTrue(TEXT("Repair accepts payment across matching resource definitions"),
		FortAthenaBuildDamage::RepairBuildingActor(Controller, Wall));
	TestNull(TEXT("Repair removes the depleted first stack"), Inventory->NativeFindItemByGuid(FirstRepairGuid));
	TestEqual(TEXT("Repair takes only the remainder of its cost from the second stack"), SecondRepairStack->ItemEntry.Count, 1);
	TestEqual(TEXT("Repair preserves unrelated materials"), ProtectedItem->ItemEntry.Count, 3);

	constexpr int32 BundleStackCount = 1 << 30;
	UFortResourceItemDefinition* BundleFirst = NewObject<UFortResourceItemDefinition>(Inventory);
	UFortResourceItemDefinition* BundleSecond = NewObject<UFortResourceItemDefinition>(Inventory);
	BundleFirst->MaxStackSize = FScalableFloat(static_cast<float>(BundleStackCount));
	BundleSecond->MaxStackSize = BundleFirst->MaxStackSize;
	FFortItemEntry FirstBundleEntry{};
	FirstBundleEntry.ItemDefinition = BundleFirst;
	FirstBundleEntry.Count = BundleStackCount;
	FFortItemEntry SecondBundleEntry = FirstBundleEntry;
	SecondBundleEntry.ItemDefinition = BundleSecond;
	AFortPickupAthena* Bundle = FortAthenaLootTransfer::SpawnTossedPickupFromEntry(
		World, FirstBundleEntry, Pawn->GetActorLocation(), FVector::ForwardVector, nullptr, 0, EFortPickupSpawnSource::Unset);
	if (!TestNotNull(TEXT("Multi-item pickup"), Bundle))
	{
		return false;
	}
	ON_SCOPE_EXIT { Bundle->Destroy(); };
	// The primary entry is only a display entry while the pickup contains a bundle.
	Bundle->MultiItemPickupEntries.Add(FFortItemEntry{});
	TestFalse(TEXT("A bundle without a valid entry grants nothing"),
		FortAthenaLootTransfer::HandlePickup(Pawn, Bundle, 0.25f, FVector::ZeroVector, false));
	TestFalse(TEXT("Retrying a rejected bundle cannot claim the display entry"),
		FortAthenaLootTransfer::HandlePickup(Pawn, Bundle, 0.25f, FVector::ZeroVector, false));
	TestNull(TEXT("The rejected display entry never reaches inventory"),
		Inventory->NativeFindWorldItemByDefinition(FirstBundleEntry.ItemDefinition, true));

	Bundle->MultiItemPickupEntries = {FirstBundleEntry, SecondBundleEntry};
	TestTrue(TEXT("Collecting a bundle with an aggregate count above int32 succeeds"),
		FortAthenaLootTransfer::HandlePickup(Pawn, Bundle, 0.25f, FVector::ZeroVector, false));
	TestTrue(TEXT("The fully collected bundle is reserved for flight"), Bundle->bPickedUp);
	TestEqual(TEXT("No bundle entries remain on the ground"), Bundle->MultiItemPickupEntries.Num(), 0);
	for (const UFortItemDefinition* BundleDefinition : {BundleFirst, BundleSecond})
	{
		const FFortItemEntry* Entry = Inventory->Inventory.ReplicatedEntries.FindByPredicate(
			[BundleDefinition](const FFortItemEntry& Candidate) { return Candidate.ItemDefinition == BundleDefinition; });
		if (TestNotNull(TEXT("The bundle entry is replicated"), Entry))
		{
			TestEqual(TEXT("The bundle entry keeps its full quantity"), Entry->Count, BundleStackCount);
		}
	}

	BundleFirst->bAllowMultipleStacks = true;
	UFortWorldItem* TargetStack = FortAthenaInventory::GiveItem(Controller, BundleFirst, BundleStackCount);
	if (!TestNotNull(TEXT("A second large stack of the same item can be granted"), TargetStack))
	{
		return false;
	}
	UFortKismetLibrary* Library = GetMutableDefault<UFortKismetLibrary>();
	UFunction* QueryFunction = Library->FindFunction(FName(TEXT("K2_GetItemQuantityOnPlayer")));
	UFunction* RemoveFunction = Library->FindFunction(FName(TEXT("K2_RemoveItemFromPlayer")));
	UFunction* RemoveByGuidFunction = Library->FindFunction(FName(TEXT("K2_RemoveItemFromPlayerByGuid")));
	UFunction* RemoveInstanceFunction = Library->FindFunction(FName(TEXT("K2_RemoveFortItemFromPlayer")));
	if (!TestNotNull(TEXT("Blueprint quantity function"), QueryFunction) ||
		!TestNotNull(TEXT("Blueprint removal function"), RemoveFunction) ||
		!TestNotNull(TEXT("Blueprint removal by GUID function"), RemoveByGuidFunction) ||
		!TestNotNull(TEXT("Blueprint removal by instance function"), RemoveInstanceFunction))
	{
		return false;
	}
	struct
	{
		AFortPlayerController* PlayerController;
		UFortItemDefinition* ItemDefinition;
		int32 ReturnValue;
	} Query{Controller, BundleFirst, 0};
	Library->ProcessEvent(QueryFunction, &Query);
	TestEqual(TEXT("Blueprint quantity saturates at the signed return limit"), Query.ReturnValue, MAX_int32);

	struct
	{
		AFortPlayerController* PlayerController;
		FGuid ItemGuid;
		int32 AmountToRemove;
		bool bForceRemoval;
		int32 ReturnValue;
	} RemoveByGuid{Controller, TargetStack->NativeGetItemGuid(), 0, false, -1};
	Library->ProcessEvent(RemoveByGuidFunction, &RemoveByGuid);
	TestEqual(TEXT("Zero-count GUID removal removes nothing"), RemoveByGuid.ReturnValue, 0);
	TestEqual(TEXT("Zero-count GUID removal preserves the stack"), TargetStack->ItemEntry.Count, BundleStackCount);
	RemoveByGuid.AmountToRemove = 2;
	Library->ProcessEvent(RemoveByGuidFunction, &RemoveByGuid);
	TestEqual(TEXT("GUID removal reports the amount removed"), RemoveByGuid.ReturnValue, 2);
	TestEqual(TEXT("GUID removal debits the selected stack"), TargetStack->ItemEntry.Count, BundleStackCount - 2);
	Library->ProcessEvent(QueryFunction, &Query);
	TestEqual(TEXT("GUID removal preserves the other matching stack"), Query.ReturnValue, MAX_int32 - 1);
	UFortWorldItem* DetachedCopy = NewObject<UFortWorldItem>(Inventory);
	DetachedCopy->ItemEntry = TargetStack->ItemEntry;
	struct
	{
		AFortPlayerController* PlayerController;
		UFortItem* Item;
		int32 AmountToRemove;
		bool bForceRemoval;
		int32 ReturnValue;
	} RemoveInstance{Controller, DetachedCopy, 1, false, -1};
	Library->ProcessEvent(RemoveInstanceFunction, &RemoveInstance);
	TestEqual(TEXT("A detached item with a matching GUID cannot debit inventory"), RemoveInstance.ReturnValue, 0);
	TestEqual(TEXT("Detached removal preserves the live stack"), TargetStack->ItemEntry.Count, BundleStackCount - 2);
	RemoveInstance.Item = TargetStack;
	Library->ProcessEvent(RemoveInstanceFunction, &RemoveInstance);
	TestEqual(TEXT("Removing the owned instance reports the debit"), RemoveInstance.ReturnValue, 1);
	TestEqual(TEXT("Removing the owned instance debits the selected stack"), TargetStack->ItemEntry.Count, BundleStackCount - 3);
	FortAthenaInventory::GiveItem(Controller, BundleFirst, 3);

	struct
	{
		AFortPlayerController* PlayerController;
		UFortWorldItemDefinition* ItemDefinition;
		int32 AmountToRemove;
		bool bForceRemoval;
		int32 ReturnValue;
	} Remove{Controller, BundleFirst, -1, false, 0};
	Library->ProcessEvent(RemoveFunction, &Remove);
	TestEqual(TEXT("Blueprint remove-all reports a saturated quantity"), Remove.ReturnValue, MAX_int32);
	Query.ReturnValue = -1;
	Library->ProcessEvent(QueryFunction, &Query);
	TestEqual(TEXT("Blueprint remove-all fully removes both matching stacks"), Query.ReturnValue, 0);
	Query.ItemDefinition = BundleSecond;
	Library->ProcessEvent(QueryFunction, &Query);
	TestEqual(TEXT("Blueprint remove-all preserves an unrelated stack"), Query.ReturnValue, BundleStackCount);

	UFortWeaponRangedItemDefinition* WideWeapon = NewObject<UFortWeaponRangedItemDefinition>(Inventory);
	WideWeapon->ItemType = EFortItemType::WeaponRanged;
	WideWeapon->MaxStackSize = FScalableFloat(1.0f);
	WideWeapon->NumberOfSlotsToTake = 2;
	auto& Slots = Controller->QuickBarDefinitions[static_cast<int32>(EFortQuickBars::Primary)].QuickbarSlots;
	Slots.Reset();
	Slots.SetNum(4);
	Slots[1].AcceptedItemTypes.Add(EFortItemType::WeaponRanged);
	Slots[2].AcceptedItemTypes.Add(EFortItemType::Consumable);
	Slots[3].AcceptedItemTypes.Add(EFortItemType::WeaponRanged);

	UFortWeaponRangedItemDefinition* BlockerDefinition = NewObject<UFortWeaponRangedItemDefinition>(Inventory);
	BlockerDefinition->ItemType = EFortItemType::WeaponRanged;
	BlockerDefinition->MaxStackSize = FScalableFloat(1.0f);
	UFortWorldItem* Blocker = FortAthenaInventory::GiveItem(Controller, BlockerDefinition, 1);
	if (!TestNotNull(TEXT("Weapon reserves the earlier slot"), Blocker))
	{
		return false;
	}

	FFortItemEntry WideEntry{};
	WideEntry.ItemDefinition = WideWeapon;
	WideEntry.Count = 1;
	AFortPickupAthena* WidePickup = FortAthenaLootTransfer::SpawnTossedPickupFromEntry(
		World, WideEntry, Pawn->GetActorLocation(), FVector::ForwardVector, nullptr, 0, EFortPickupSpawnSource::Unset);
	if (!TestNotNull(TEXT("Two-slot weapon pickup"), WidePickup))
	{
		return false;
	}
	ON_SCOPE_EXIT { WidePickup->Destroy(); };
	TestFalse(TEXT("An incompatible middle slot blocks the two-slot pickup"),
		FortAthenaLootTransfer::HandlePickup(Pawn, WidePickup, 0.25f, FVector::ZeroVector, false));
	TestNull(TEXT("Rejected pickup creates no inventory stack"), Inventory->NativeFindWorldItemByDefinition(WideWeapon, true));
	TestEqual(TEXT("Rejected pickup retains its world count"), WidePickup->PrimaryPickupItemEntry.Count, 1);
	Slots[2].AcceptedItemTypes.Add(EFortItemType::WeaponRanged);
	TestTrue(TEXT("The same pickup succeeds once contiguous slots accept it"),
		FortAthenaLootTransfer::HandlePickup(Pawn, WidePickup, 0.25f, FVector::ZeroVector, false));
	UFortWorldItem* WideItem = Inventory->NativeFindWorldItemByDefinition(WideWeapon, true);
	AFortWeapon* HeldWeapon = World->SpawnActor<AFortWeapon>();
	ON_SCOPE_EXIT
	{
		if (HeldWeapon)
		{
			Pawn->CurrentWeapon = nullptr;
			HeldWeapon->Destroy();
		}
	};
	if (!TestNotNull(TEXT("Accepted pickup grants the two-slot weapon"), WideItem) ||
		!TestNotNull(TEXT("Held weapon for swap fallback"), HeldWeapon))
	{
		return false;
	}
	TestEqual(TEXT("Removing the blocker frees an earlier slot before the swap"),
		FortAthenaInventory::RemoveItem(Controller, Blocker->NativeGetItemGuid(), 1), 1);
	WideWeapon->bCanBeDropped = true;
	WideWeapon->DropBehavior = EWorldItemDropBehavior::DestroyOnDrop;
	const FGuid HeldGuid = WideItem->NativeGetItemGuid();
	HeldWeapon->ItemEntryGuid = HeldGuid;
	Pawn->CurrentWeapon = HeldWeapon;
	AFortPickupAthena* SwapPickup = FortAthenaLootTransfer::SpawnTossedPickupFromEntry(
		World, WideEntry, Pawn->GetActorLocation(), FVector::ForwardVector, nullptr, 0, EFortPickupSpawnSource::Unset);
	if (!TestNotNull(TEXT("Replacement pickup"), SwapPickup))
	{
		return false;
	}
	ON_SCOPE_EXIT { SwapPickup->Destroy(); };
	TestFalse(TEXT("A stale swap GUID cannot substitute the held weapon"),
		FortAthenaLootTransfer::HandlePickupWithSwap(Pawn, SwapPickup, FGuid::NewGuid(), 0.25f, FVector::ZeroVector, false, true));
	TestTrue(TEXT("Rejected swap preserves the held stack"), Inventory->NativeFindItemByGuid(HeldGuid) == WideItem);
	TestEqual(TEXT("Rejected swap preserves the world pickup"), SwapPickup->PrimaryPickupItemEntry.Count, 1);
	WideWeapon->bCanBeDropped = false;
	TestFalse(TEXT("A protected held item cannot be swapped even by its exact GUID"),
		FortAthenaLootTransfer::HandlePickupWithSwap(Pawn, SwapPickup, HeldGuid, 0.25f, FVector::ZeroVector, false, true));
	TestTrue(TEXT("Rejected protected swap preserves its inventory item"), Inventory->NativeFindItemByGuid(HeldGuid) == WideItem);
	WideWeapon->bCanBeDropped = true;
	WideWeapon->DropBehavior = EWorldItemDropBehavior::DropAsPickup;
	AFortPickupAthena* OutgoingPickup = nullptr;
	const FDelegateHandle SpawnCallback = World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateLambda([&](AActor* Actor)
	{
		if (!OutgoingPickup)
		{
			OutgoingPickup = Cast<AFortPickupAthena>(Actor);
			if (OutgoingPickup)
			{
				TestFalse(TEXT("A swap spawn callback cannot claim the incoming pickup again"),
					FortAthenaLootTransfer::HandlePickupWithSwap(Pawn, SwapPickup, HeldGuid, 0.25f, FVector::ZeroVector, false, true));
			}
		}
	}));
	TestTrue(TEXT("A valid explicit swap accepts the replacement"),
		FortAthenaLootTransfer::HandlePickupWithSwap(Pawn, SwapPickup, HeldGuid, 0.25f, FVector::ZeroVector, false, true));
	World->RemoveOnActorSpawnedHandler(SpawnCallback);
	if (TestNotNull(TEXT("The swap spawns outgoing loot"), OutgoingPickup))
	{
		TestEqual(TEXT("The outgoing pickup holds exactly one weapon"), OutgoingPickup->PrimaryPickupItemEntry.Count, 1);
		OutgoingPickup->Destroy();
	}
	TestNull(TEXT("The swap removes the exact outgoing inventory GUID"), Inventory->NativeFindItemByGuid(HeldGuid));
	const UFortWorldItem* Replacement = Inventory->NativeFindWorldItemByDefinition(WideWeapon, true);
	if (TestNotNull(TEXT("The replacement is granted"), Replacement))
	{
		TestEqual(TEXT("The replacement contains one item"), Replacement->ItemEntry.Count, 1);
		TestTrue(TEXT("The replacement keeps the swapped slot when an earlier slot also fits"),
			Controller->DelayedQuickBarActions.Items.ContainsByPredicate([&](const FDelayedQuickBarAction& Action)
			{
				return Action.Action == EFortDelayedQuickBarAction::Add && Action.ItemGuid == Replacement->ItemEntry.ItemGuid &&
					Action.QuickBarType == EFortQuickBars::Primary && Action.QuickBarSlot == 2;
			}));
	}
	TestEqual(TEXT("The replacement pickup is depleted"), SwapPickup->PrimaryPickupItemEntry.Count, 0);
	if (!Replacement)
	{
		return false;
	}

	ABuildingItemWeaponUpgradeActor* Bench = World->SpawnActor<ABuildingItemWeaponUpgradeActor>();
	if (!TestNotNull(TEXT("Weapon upgrade bench fixture"), Bench))
	{
		return false;
	}
	ON_SCOPE_EXIT { Bench->Destroy(); };
	if (!Bench->GetRootComponent())
	{
		USceneComponent* Root = NewObject<USceneComponent>(Bench);
		Bench->SetRootComponent(Root);
		Root->RegisterComponent();
	}
	Bench->SetActorLocation(Pawn->GetActorLocation());
	Bench->UpgradeTable = NewObject<UDataTable>(Bench);
	Bench->UpgradeTable->RowStruct = FWeaponUpgradeItemRow::StaticStruct();
	UFortWeaponRangedItemDefinition* UpgradedDefinition = NewObject<UFortWeaponRangedItemDefinition>(Inventory);
	UpgradedDefinition->ItemType = EFortItemType::WeaponRanged;
	UpgradedDefinition->MaxStackSize = FScalableFloat(1.0f);
	UpgradedDefinition->WeaponActorClass = AFortWeapon::StaticClass();
	FWeaponUpgradeItemRow Recipe;
	Recipe.CurrentWeaponDef = WideWeapon;
	Recipe.UpgradedWeaponDef = UpgradedDefinition;
	Recipe.Direction = EFortWeaponUpgradeDirection::Vertical;
	Recipe.WoodCost = EFortWeaponUpgradeCosts::WoodUncommon;
	Recipe.MetalCost = EFortWeaponUpgradeCosts::MetalUncommon;
	Recipe.BrickCost = EFortWeaponUpgradeCosts::BrickUncommon;
	Bench->UpgradeTable->AddRow(TEXT("Upgrade"), Recipe);
	UFortResourceItemDefinition* Wood = NewObject<UFortResourceItemDefinition>(Inventory);
	Wood->MaxStackSize = FScalableFloat(4.0f);
	Wood->bAllowMultipleStacks = true;
	UFortResourceItemDefinition* Metal = NewObject<UFortResourceItemDefinition>(Inventory);
	UFortResourceItemDefinition* Brick = NewObject<UFortResourceItemDefinition>(Inventory);
	Metal->MaxStackSize = FScalableFloat(10.0f);
	Brick->MaxStackSize = FScalableFloat(10.0f);
	Bench->WoodItem = Wood;
	Bench->MetalItem = Metal;
	Bench->BrickItem = Brick;
	Bench->WoodCostCurve = FScalableFloat(5.0f);
	Bench->MetalCostCurve = FScalableFloat(2.0f);
	Bench->BrickCostCurve = FScalableFloat(1.0f);
	FortAthenaInventory::GiveItem(Controller, Wood, 4);
	FortAthenaInventory::GiveItem(Controller, Wood, 3);
	UFortWorldItem* MetalStack = FortAthenaInventory::GiveItem(Controller, Metal, 5);
	if (!TestNotNull(TEXT("Bench metal stack"), MetalStack))
	{
		return false;
	}

	const FGuid UpgradeSource = Replacement->ItemEntry.ItemGuid;
	HeldWeapon->ItemEntryGuid = UpgradeSource;
	Pawn->CurrentWeapon = HeldWeapon;
	UFortControllerComponent_Interaction* Interaction = NewObject<UFortControllerComponent_Interaction>(Controller);
	Interaction->RegisterComponent();
	Interaction->ServerAttemptInteract(Bench, nullptr, TInteractionType::IT_LongPress, nullptr,
		EInteractionBeingAttempted::FirstInteraction, 1);
	TestTrue(TEXT("An unaffordable upgrade preserves the equipped inventory item"),
		Inventory->NativeFindItemByGuid(UpgradeSource) == Replacement);
	TestEqual(TEXT("An unaffordable upgrade preserves metal"), MetalStack->ItemEntry.Count, 5);
	TestNull(TEXT("An unaffordable upgrade creates no result"), Inventory->NativeFindWorldItemByDefinition(UpgradedDefinition, true));

	FortAthenaInventory::GiveItem(Controller, Brick, 1);
	Interaction->ServerAttemptInteract(Bench, nullptr, TInteractionType::IT_LongPress, nullptr,
		EInteractionBeingAttempted::FirstInteraction, 2);
	TestNull(TEXT("A paid upgrade removes the equipped inventory item"), Inventory->NativeFindItemByGuid(UpgradeSource));
	TestEqual(TEXT("A paid upgrade spends exactly two metal"), MetalStack->ItemEntry.Count, 3);
	const UFortWorldItem* WoodRemainder = Inventory->NativeFindWorldItemByDefinition(Wood, true);
	if (TestNotNull(TEXT("Wood remaining after spending from two stacks"), WoodRemainder))
	{
		TestEqual(TEXT("A paid upgrade spends exactly five wood"), WoodRemainder->ItemEntry.Count, 2);
	}
	TestNull(TEXT("A paid upgrade spends the last brick"), Inventory->NativeFindWorldItemByDefinition(Brick, true));
	const UFortWorldItem* Upgraded = Inventory->NativeFindWorldItemByDefinition(UpgradedDefinition, true);
	if (TestNotNull(TEXT("The upgrade grants its result"), Upgraded))
	{
		TestTrue(TEXT("The server equips the upgrade result"),
			Pawn->CurrentWeapon && Pawn->CurrentWeapon->GetInventoryGUID() == Upgraded->ItemEntry.ItemGuid);
		TestTrue(TEXT("The upgrade result uses the equipped weapon's slot"),
			Controller->DelayedQuickBarActions.Items.ContainsByPredicate([&](const FDelayedQuickBarAction& Action)
			{
				return Action.Action == EFortDelayedQuickBarAction::Add && Action.ItemGuid == Upgraded->ItemEntry.ItemGuid &&
					Action.QuickBarType == EFortQuickBars::Primary && Action.QuickBarSlot == 2;
			}));
	}

	const UClass* CookedBenchClass = LoadClass<ABuildingItemWeaponUpgradeActor>(nullptr,
		TEXT("/Game/Athena/Items/Gameplay/Wumba/B_Athena_Wumba.B_Athena_Wumba_C"));
	const ABuildingItemWeaponUpgradeActor* CookedBench = CookedBenchClass
		? CookedBenchClass->GetDefaultObject<ABuildingItemWeaponUpgradeActor>() : nullptr;
	if (!TestNotNull(TEXT("Cooked bench defaults for the interaction fixture"), CookedBench) ||
		!TestNotNull(TEXT("Cooked bench recipe table"), CookedBench->UpgradeTable))
	{
		return false;
	}
	const FWeaponUpgradeItemRow* Sidegrade = CookedBench->UpgradeTable->FindRow<FWeaponUpgradeItemRow>(
		TEXT("1001"), TEXT("Sidegrade interaction test"));
	if (!TestNotNull(TEXT("Cooked rifle sidegrade"), Sidegrade) ||
		!TestNotNull(TEXT("Cooked sidegrade source"), Sidegrade->CurrentWeaponDef) ||
		!TestNotNull(TEXT("Cooked sidegrade result"), Sidegrade->UpgradedWeaponDef))
	{
		return false;
	}
	Bench->UpgradeTable = CookedBench->UpgradeTable;
	Bench->HorizontalWoodCostCurve = CookedBench->HorizontalWoodCostCurve;
	Bench->HorizontalMetalCostCurve = CookedBench->HorizontalMetalCostCurve;
	Bench->HorizontalBrickCostCurve = CookedBench->HorizontalBrickCostCurve;
	Bench->WoodItem = CookedBench->WoodItem;
	Bench->MetalItem = CookedBench->MetalItem;
	Bench->BrickItem = CookedBench->BrickItem;
	UFortWorldItem* Rifle = FortAthenaInventory::GiveItem(Controller, Sidegrade->CurrentWeaponDef, 1);
	UFortWorldItem* SidegradeWood = FortAthenaInventory::GiveItem(Controller, Bench->WoodItem, 25);
	UFortWorldItem* SidegradeMetal = FortAthenaInventory::GiveItem(Controller, Bench->MetalItem, 26);
	UFortWorldItem* SidegradeBrick = FortAthenaInventory::GiveItem(Controller, Bench->BrickItem, 27);
	if (!TestNotNull(TEXT("Cooked rifle inventory entry"), Rifle) ||
		!TestNotNull(TEXT("Cooked wood"), SidegradeWood) ||
		!TestNotNull(TEXT("Cooked metal"), SidegradeMetal) ||
		!TestNotNull(TEXT("Cooked brick"), SidegradeBrick))
	{
		return false;
	}
	const FGuid RifleGuid = Rifle->NativeGetItemGuid();
	TestTrue(TEXT("The cooked rifle receives a primary quickbar slot"),
		Controller->DelayedQuickBarActions.Items.ContainsByPredicate([&](const FDelayedQuickBarAction& Action)
		{
			return Action.Action == EFortDelayedQuickBarAction::Add && Action.ItemGuid == RifleGuid &&
				Action.QuickBarType == EFortQuickBars::Primary;
		}));
	HeldWeapon->ItemEntryGuid = RifleGuid;
	Pawn->CurrentWeapon = HeldWeapon;
	{
		// The dedicated fixture lacks the cooked actor's audiovisual dependencies.
		TGuardValue<TSoftClassPtr<AFortWeapon>> ActorClass(Sidegrade->UpgradedWeaponDef->WeaponActorClass,
			TSoftClassPtr<AFortWeapon>(AFortWeapon::StaticClass()));
		Interaction->ServerAttemptInteract(Bench, nullptr, TInteractionType::IT_LongPress, nullptr,
			EInteractionBeingAttempted::SecondInteraction, 3);
	}
	TestNull(TEXT("The second interaction consumes the original rifle"), Inventory->NativeFindItemByGuid(RifleGuid));
	TestEqual(TEXT("The cooked sidegrade charges 20 wood"), SidegradeWood->ItemEntry.Count, 5);
	TestEqual(TEXT("The cooked sidegrade charges 20 metal"), SidegradeMetal->ItemEntry.Count, 6);
	TestEqual(TEXT("The cooked sidegrade charges 20 brick"), SidegradeBrick->ItemEntry.Count, 7);
	const UFortWorldItem* HeavyRifle = Inventory->NativeFindWorldItemByDefinition(Sidegrade->UpgradedWeaponDef, true);
	if (TestNotNull(TEXT("The second interaction grants the cooked heavy rifle"), HeavyRifle))
	{
		TestTrue(TEXT("The sidegrade equips the resulting inventory GUID"),
			Pawn->CurrentWeapon && Pawn->CurrentWeapon->GetInventoryGUID() == HeavyRifle->ItemEntry.ItemGuid);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FServerWeaponUpgradeRecipeTest, "Fortnite.Server.Inventory.WeaponUpgradeRecipe",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)

bool FServerWeaponUpgradeRecipeTest::RunTest(const FString&)
{
	UDataTable* Table = NewObject<UDataTable>();
	Table->RowStruct = FWeaponUpgradeItemRow::StaticStruct();
	UFortWeaponRangedItemDefinition* Source = NewObject<UFortWeaponRangedItemDefinition>(Table);
	UFortWeaponRangedItemDefinition* Upgrade = NewObject<UFortWeaponRangedItemDefinition>(Table);
	UFortWeaponRangedItemDefinition* Sidegrade = NewObject<UFortWeaponRangedItemDefinition>(Table);

	FWeaponUpgradeItemRow Vertical;
	Vertical.CurrentWeaponDef = Source;
	Vertical.UpgradedWeaponDef = Upgrade;
	Vertical.Direction = EFortWeaponUpgradeDirection::Vertical;
	Vertical.WoodCost = EFortWeaponUpgradeCosts::WoodRare;
	Table->AddRow(TEXT("Upgrade"), Vertical);
	Table->AddRow(TEXT("DuplicateUpgrade"), Vertical);

	FWeaponUpgradeItemRow Horizontal = Vertical;
	Horizontal.UpgradedWeaponDef = Sidegrade;
	Horizontal.Direction = EFortWeaponUpgradeDirection::Horizontal;
	Horizontal.WoodCost = EFortWeaponUpgradeCosts::HorizontalWoodUncommon;
	Table->AddRow(TEXT("Sidegrade"), Horizontal);

	const FWeaponUpgradeItemRow* Recipe = FWeaponUpgradeItemRow::FindRecipe(Table, Source, Vertical.Direction);
	if (TestNotNull(TEXT("Identical upgrade rows resolve to a single recipe"), Recipe))
	{
		TestTrue(TEXT("Vertical recipe selects the upgrade weapon"), Recipe->UpgradedWeaponDef == Upgrade);
		TestTrue(TEXT("Vertical recipe preserves its material cost selector"), Recipe->WoodCost == Vertical.WoodCost);
	}
	Recipe = FWeaponUpgradeItemRow::FindRecipe(Table, Source, Horizontal.Direction);
	if (TestNotNull(TEXT("Horizontal recipe resolves separately"), Recipe))
	{
		TestTrue(TEXT("Horizontal recipe selects the sidegrade weapon"), Recipe->UpgradedWeaponDef == Sidegrade);
		TestTrue(TEXT("Horizontal recipe preserves its material cost selector"), Recipe->WoodCost == Horizontal.WoodCost);
	}
	TestNull(TEXT("A different weapon has no recipe"), FWeaponUpgradeItemRow::FindRecipe(Table, Upgrade, Vertical.Direction));
	TestNull(TEXT("An unset direction has no recipe"),
		FWeaponUpgradeItemRow::FindRecipe(Table, Source, EFortWeaponUpgradeDirection::NotSet));

	FWeaponUpgradeItemRow Invalid = Vertical;
	Invalid.Direction = static_cast<EFortWeaponUpgradeDirection>(3);
	Table->AddRow(TEXT("InvalidDirection"), Invalid);
	TestNull(TEXT("An unsupported direction has no recipe even when present in the table"),
		FWeaponUpgradeItemRow::FindRecipe(Table, Source, Invalid.Direction));

	Vertical.WoodCost = EFortWeaponUpgradeCosts::WoodVeryRare;
	Table->AddRow(TEXT("ConflictingCost"), Vertical);
	TestNull(TEXT("Conflicting prices reject the recipe even when the result weapon matches"),
		FWeaponUpgradeItemRow::FindRecipe(Table, Source, Vertical.Direction));

	if (IsRunningDedicatedServer() && FortAthenaInventory::IsEnabled())
	{
		const UDataTable* CookedTable = LoadObject<UDataTable>(nullptr,
			TEXT("/Game/Items/Datatables/AthenaWumbaData.AthenaWumbaData"));
		if (!TestNotNull(TEXT("Cooked upgrade recipes"), CookedTable))
		{
			return false;
		}
		const FWeaponUpgradeItemRow* Pistol = CookedTable->FindRow<FWeaponUpgradeItemRow>(TEXT("62"), TEXT("Inventory test"));
		const FWeaponUpgradeItemRow* Rifle = CookedTable->FindRow<FWeaponUpgradeItemRow>(TEXT("1001"), TEXT("Inventory test"));
		if (!TestNotNull(TEXT("Cooked pistol upgrade"), Pistol) || !TestNotNull(TEXT("Cooked rifle sidegrade"), Rifle))
		{
			return false;
		}
		TestTrue(TEXT("Cooked pistol recipe resolves even when its price tier differs from the result rarity"),
			Pistol->UpgradedWeaponDef && FWeaponUpgradeItemRow::FindRecipe(CookedTable, Pistol->CurrentWeaponDef,
				EFortWeaponUpgradeDirection::Vertical) == Pistol);
		TestTrue(TEXT("Cooked pistol uses the Very Rare material selectors"),
			Pistol->WoodCost == EFortWeaponUpgradeCosts::WoodVeryRare &&
			Pistol->MetalCost == EFortWeaponUpgradeCosts::MetalVeryRare &&
			Pistol->BrickCost == EFortWeaponUpgradeCosts::BrickVeryRare);
		TestTrue(TEXT("Cooked rifle resolves to its horizontal recipe"),
			Rifle->UpgradedWeaponDef && FWeaponUpgradeItemRow::FindRecipe(CookedTable, Rifle->CurrentWeaponDef,
				EFortWeaponUpgradeDirection::Horizontal) == Rifle);
		TestTrue(TEXT("Cooked rifle uses the Common horizontal material selectors"),
			Rifle->WoodCost == EFortWeaponUpgradeCosts::HorizontalWoodCommon &&
			Rifle->MetalCost == EFortWeaponUpgradeCosts::HorizontalMetalCommon &&
			Rifle->BrickCost == EFortWeaponUpgradeCosts::HorizontalBrickCommon);

		const UClass* BenchClass = LoadClass<ABuildingItemWeaponUpgradeActor>(nullptr,
			TEXT("/Game/Athena/Items/Gameplay/Wumba/B_Athena_Wumba.B_Athena_Wumba_C"));
		const ABuildingItemWeaponUpgradeActor* Bench = BenchClass ? BenchClass->GetDefaultObject<ABuildingItemWeaponUpgradeActor>() : nullptr;
		if (!TestNotNull(TEXT("Cooked upgrade bench defaults"), Bench))
		{
			return false;
		}
		FIntVector Costs(-1, -1, -1);
		TestTrue(TEXT("Pistol upgrade resolves its cooked prices"), Pistol->TryResolveMaterialCosts(*Bench, Costs));
		TestTrue(TEXT("The tier-three pistol upgrade costs 150 of each material"), Costs == FIntVector(150, 150, 150));
		TestTrue(TEXT("Rifle sidegrade resolves its cooked prices"), Rifle->TryResolveMaterialCosts(*Bench, Costs));
		TestTrue(TEXT("The tier-zero sidegrade costs 20 of each material"), Costs == FIntVector(20, 20, 20));
		FWeaponUpgradeItemRow BadRecipe = *Rifle;
		BadRecipe.BrickCost = EFortWeaponUpgradeCosts::HorizontalMetalCommon;
		TestFalse(TEXT("A selector for the wrong material rejects the price"), BadRecipe.TryResolveMaterialCosts(*Bench, Costs));
		TestTrue(TEXT("Failed price lookup leaves the previous quote intact"), Costs == FIntVector(20, 20, 20));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FServerConsumableCostDataTest, "Fortnite.Server.Inventory.ConsumableCostData",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)

bool FServerConsumableCostDataTest::RunTest(const FString&)
{
	// The headless stage omits audiovisual packages referenced by these cooked assets.
	AddExpectedError(TEXT("Couldn't find file for package (/Game/Athena/Sounds/Items/Item_Consumables_((Bandage|Medkit)_Apply_Cue|Shield_Potion_(Mini_)?Drink(_Complete)?_Cue)|/Game/Effects/Fort_Effects/Effects/Characters/Generic/Consumables/P_Consumable_(ItemUseComplete(_Blue)?|MedKit_PourDecal)|/Game/Packages/Fortress_SharedMaterials/Base_Material/DEMO_Master|/Game/Sounds/Fort_GamePlay_Sounds/Loot/EatHealth_Restore_Athena_Cue) "), EAutomationExpectedErrorFlags::Contains, 0);
	AddExpectedError(TEXT("Found 1 dependent packages"), EAutomationExpectedErrorFlags::Contains, 0);
	AddExpectedError(TEXT("  /Game/(Animation/Game/MainPlayer/Montages/General/(ApplyMedkit|WrapBandages|ApplySmallShield|ApplyShield)_Athena|Athena/Items/Consumables/Bandage/BandageCuff_Bandage_Roll_LOD0)$"), EAutomationExpectedErrorFlags::Contains, 0);
	AddExpectedError(TEXT("Attempt to build nav collision data for /Game/Athena/Items/Consumables/Bandage/BandageCuff_Bandage_Roll_LOD0"), EAutomationExpectedErrorFlags::Contains, 0);

	UFortWeaponItemDefinition* Bandage = LoadObject<UFortWeaponItemDefinition>(nullptr,
		TEXT("/Game/Athena/Items/Consumables/Bandage/Athena_Bandage.Athena_Bandage"));
	if (!TestNotNull(TEXT("Cooked bandage item"), Bandage))
	{
		return false;
	}
	TestEqual(TEXT("Bandage stack cap resolves from game data"), Bandage->NativeGetMaxStackSize(nullptr), 15);
	TestTrue(TEXT("Bandage uses its inventory definition as ammo"), Bandage->GetAmmoWorldItemDefinitionNative() == Bandage);
	TestEqual(TEXT("Bandage loot has no separate magazine"), FortAthenaLootSearch::ResolveLoadedAmmo(Bandage), 0);

	const UClass* AbilityClass = Bandage->GetPrimaryFireAbility();
	const UFortGameplayAbility* Ability = AbilityClass ? AbilityClass->GetDefaultObject<UFortGameplayAbility>() : nullptr;
	if (!TestNotNull(TEXT("Cooked bandage primary ability"), Ability))
	{
		return false;
	}
	const FFortAbilityCost* Cost = Ability->AbilityCosts.FindByPredicate([](const FFortAbilityCost& Entry)
	{
		return Entry.CostSource == EFortAbilityCostSource::AmmoPrimary && !Entry.bOnlyApplyCostOnHit;
	});
	if (TestNotNull(TEXT("Bandage charges primary ammo at commit"), Cost))
	{
		TestEqual(TEXT("Bandage cost is one unit"), Cost->CostValue.GetValueAtLevel(1.0f), 1.0f);
	}
	if (!IsRunningDedicatedServer() || !FortAthenaInventory::IsEnabled())
	{
		return true;
	}
	UWorld* World = nullptr;
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.World() && Context.World()->GetNetMode() == NM_DedicatedServer)
		{
			World = Context.World();
			break;
		}
	}
	if (!TestNotNull(TEXT("Dedicated server world"), World))
	{
		return false;
	}

	AFortPlayerControllerAthena* Controller = World->SpawnActor<AFortPlayerControllerAthena>();
	AFortInventory* Inventory = World->SpawnActor<AFortInventory>();
	AFortWeapon* Weapon = World->SpawnActor<AFortWeapon>();
	AFortPlayerPawnAthena* Pawn = nullptr;
	bool bCleanupQueued = false;
	ON_SCOPE_EXIT
	{
		if (Weapon)
		{
			Weapon->Destroy();
		}
		if (bCleanupQueued)
		{
			return;
		}
		if (Pawn)
		{
			Pawn->Destroy();
		}
		if (Controller)
		{
			Controller->WorldInventory = nullptr;
			Controller->Destroy();
		}
		if (Inventory)
		{
			Inventory->Destroy();
		}
	};
	if (!TestNotNull(TEXT("Consumable controller"), Controller) ||
		!TestNotNull(TEXT("Consumable inventory"), Inventory) || !TestNotNull(TEXT("Consumable source"), Weapon))
	{
		return false;
	}
	Controller->WorldInventory = Inventory;
	UFortWorldItem* Item = FortAthenaInventory::GiveItem(Controller, Bandage, 1, 1, 5);
	if (!TestNotNull(TEXT("Bandage stack"), Item))
	{
		return false;
	}
	TestEqual(TEXT("Bandage grants clear legacy magazine values"), Item->ItemEntry.LoadedAmmo, 0);
	TestTrue(TEXT("A legacy bandage grant can top up the same stack"),
		FortAthenaInventory::GiveItem(Controller, Bandage, 1, 1, 5) == Item);
	TestEqual(TEXT("Both bandage units are in the same stack"), Item->ItemEntry.Count, 2);
	const FGuid Guid = Item->NativeGetItemGuid();
	Weapon->WeaponData = Bandage;
	Weapon->ItemEntryGuid = Guid;

	UAbilitySystemComponent* ASC = NewObject<UAbilitySystemComponent>(Controller);
	ASC->RegisterComponent();
	ASC->InitAbilityActorInfo(Controller, Controller);
	FGameplayAbilityActorInfo ActorInfo;
	ActorInfo.InitFromActor(Controller, Controller, ASC);
	ActorInfo.PlayerController = Controller;
	const FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(
		FGameplayAbilitySpec(Ability->GetClass(), 1, INDEX_NONE, Weapon));
	if (!TestTrue(TEXT("Bandage ability granted"), Handle.IsValid()))
	{
		return false;
	}

	UFortGameplayAbility* AbilityCDO = Ability->GetClass()->GetDefaultObject<UFortGameplayAbility>();
	const FGameplayAbilityActivationInfo ActivationInfo(Controller);
	TestTrue(TEXT("Commit spends one bandage"), AbilityCDO->CommitAbilityCost(Handle, &ActorInfo, ActivationInfo));
	TestEqual(TEXT("One bandage remains"), Item->ItemEntry.Count, 1);
	ActorInfo.PlayerController.Reset();
	TestTrue(TEXT("Commit can spend the last bandage through the ability owner"),
		AbilityCDO->CommitAbilityCost(Handle, &ActorInfo, ActivationInfo));
	TestNull(TEXT("Depleted source stack is removed"), Inventory->NativeFindItemByGuid(Guid));
	TestFalse(TEXT("Commit fails after the source stack is gone"), AbilityCDO->CommitAbilityCost(Handle, &ActorInfo, ActivationInfo));

	UFortResourceItemDefinition* OtherDefinition = NewObject<UFortResourceItemDefinition>(Inventory);
	OtherDefinition->MaxStackSize = FScalableFloat(10.0f);
	UFortWorldItem* OtherItem = FortAthenaInventory::GiveItem(Controller, OtherDefinition, 2);
	if (!TestNotNull(TEXT("Unrelated inventory stack"), OtherItem))
	{
		return false;
	}
	Weapon->ItemEntryGuid = OtherItem->NativeGetItemGuid();
	TestFalse(TEXT("Commit rejects an unrelated source stack"), AbilityCDO->CommitAbilityCost(Handle, &ActorInfo, ActivationInfo));
	AbilityCDO->ApplyCost(Handle, &ActorInfo, ActivationInfo);
	TestEqual(TEXT("Applying the cost preserves the unrelated stack"), OtherItem->ItemEntry.Count, 2);
	Pawn = World->SpawnActor<AFortPlayerPawnAthena>(FVector(0, 0, 16000), FRotator::ZeroRotator);
	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(Controller->PlayerState);
	if (!TestNotNull(TEXT("Consumable pawn"), Pawn) || !TestNotNull(TEXT("Consumable player state"), PlayerState))
	{
		return false;
	}
	Controller->Possess(Pawn);
	UFortAbilitySystemComponent* PawnASC = NewObject<UFortAbilitySystemComponent>(PlayerState);
	PlayerState->AbilitySystemComponent = PawnASC;
	Pawn->AbilitySystemComponent = PawnASC;
	PawnASC->bSuppressGameplayCues = true;
	PawnASC->RegisterComponent();
	PawnASC->InitAbilityActorInfo(PlayerState, Pawn);
	Pawn->HealthSet = NewObject<UFortHealthSet>(Pawn);
	PawnASC->AddAttributeSetSubobject(Pawn->HealthSet);
	Pawn->bHealthSynced = true;
	UServerRecoveryEffect::BindHealingAttribute(*PawnASC);
	Pawn->SetMaxHealth(100.0f);
	Pawn->SetHealth(50.0f);

	UFortWorldItem* CancelledBandage = FortAthenaInventory::GiveItem(Controller, Bandage, 1);
	if (!TestNotNull(TEXT("Bandage stack for cancellation"), CancelledBandage))
	{
		return false;
	}
	const FGuid CancelledGuid = CancelledBandage->NativeGetItemGuid();
	Weapon->SetOwner(Pawn);
	Weapon->SetInstigator(Pawn);
	Weapon->ItemEntryGuid = CancelledGuid;
	Pawn->CurrentWeapon = Weapon;
	Pawn->CurrentWeaponList.Add(Weapon);
	const TArray<FortAthenaCombatEquip::FGrant> Grants = {{Ability->GetClass(), 0}};
	if (!TestTrue(TEXT("Cancellation fixture grants the primary ability"),
		FortAthenaCombatEquip::EnsureGrants(PlayerState, Pawn, Weapon, Grants, nullptr, false)))
	{
		return false;
	}
	{
		TGuardValue<TWeakObjectPtr<APlayerController>> LocalActivation(
			PawnASC->AbilityActorInfo->PlayerController, TWeakObjectPtr<APlayerController>());
		if (!TestTrue(TEXT("Cancellation fixture activates the primary ability"),
			PawnASC->TryActivateAbility(Weapon->PrimaryAbilitySpecHandle, false)))
		{
			return false;
		}
	}
	bool bSawAbilityEnd = false;
	const FDelegateHandle EndCallback = PawnASC->AbilityEndedCallbacks.AddLambda([&](UGameplayAbility*)
	{
		bSawAbilityEnd = true;
		TestNull(TEXT("Ability end callback sees the depleted item detached"), Inventory->NativeFindItemByGuid(CancelledGuid));
		TestFalse(TEXT("Ability end callback sees the replicated entry removed"),
			Inventory->Inventory.ReplicatedEntries.ContainsByPredicate([&](const FFortItemEntry& Entry)
			{
				return Entry.ItemGuid == CancelledGuid;
			}));
		TestEqual(TEXT("Ability end callback cannot remove the same item again"),
			FortAthenaInventory::RemoveItem(Controller, CancelledGuid, 1), 0);
	});
	TestEqual(TEXT("Removing the active bandage debits one unit"),
		FortAthenaInventory::RemoveItem(Controller, CancelledGuid, 1), 1);
	PawnASC->AbilityEndedCallbacks.Remove(EndCallback);
	TestTrue(TEXT("Removing the active bandage ends its granted ability"), bSawAbilityEnd);
	Pawn->CurrentWeaponList.Remove(Weapon);
	Pawn->CurrentWeapon = nullptr;

	UFortWorldItem* LastBandage = FortAthenaInventory::GiveItem(Controller, Bandage, 1);
	if (!TestNotNull(TEXT("Last bandage for activation"), LastBandage))
	{
		return false;
	}
	FFortItemEntry LegacyBandageEntry = LastBandage->ItemEntry;
	LegacyBandageEntry.LoadedAmmo = 5;
	AFortPickupAthena* DroppedBandage = FortAthenaLootTransfer::SpawnTossedPickupFromEntry(
		World, LegacyBandageEntry, Pawn->GetActorLocation(), FVector::ForwardVector, Pawn, 0, EFortPickupSpawnSource::Unset);
	if (!TestNotNull(TEXT("Dropped legacy bandage"), DroppedBandage))
	{
		return false;
	}
	TestEqual(TEXT("Dropped bandage keeps its charge"), DroppedBandage->PrimaryPickupItemEntry.Count, 1);
	TestEqual(TEXT("Dropped bandage has no magazine ammo"), DroppedBandage->PrimaryPickupItemEntry.LoadedAmmo, 0);
	DroppedBandage->Destroy();

	const FGuid LastBandageGuid = LastBandage->NativeGetItemGuid();
	// Use a native source actor because the staged cooked weapon mesh cannot evaluate its animation codec.
	AFortWeapon* Equipped = World->SpawnActor<AFortWeapon>();
	if (!TestNotNull(TEXT("Bandage ability source actor"), Equipped))
	{
		return false;
	}
	Equipped->SetOwner(Pawn);
	Equipped->SetInstigator(Pawn);
	Equipped->WeaponData = Bandage;
	Equipped->ItemEntryGuid = LastBandageGuid;
	Pawn->CurrentWeapon = Equipped;
	Pawn->CurrentWeaponList.Add(Equipped);
	// Grant the cooked primary ability directly so the headless test can run with the inventory ability audit disabled.
	const FGameplayAbilitySpecHandle PrimaryHandle = PawnASC->GiveAbility(
		FGameplayAbilitySpec(Ability->GetClass(), 1, INDEX_NONE, Equipped));
	if (!TestTrue(TEXT("Cooked bandage ability is granted"), PrimaryHandle.IsValid()))
	{
		Pawn->CurrentWeaponList.Remove(Equipped);
		Pawn->CurrentWeapon = nullptr;
		Equipped->Destroy();
		return false;
	}
	TestFalse(TEXT("Reload cannot spend a self-ammo item"), FortWeaponReload::LoadMagazine(Equipped));
	TestEqual(TEXT("The bandage still has one charge before use"), LastBandage->ItemEntry.Count, 1);
	{
		// Use the headless GAS authority path for a locally predicted ability.
		TGuardValue<TWeakObjectPtr<APlayerController>> LocalActivation(
			PawnASC->AbilityActorInfo->PlayerController, TWeakObjectPtr<APlayerController>());
		if (!TestTrue(TEXT("Equipped bandage ability activates"), PawnASC->TryActivateAbility(PrimaryHandle, false)))
		{
			return false;
		}
	}

	UFortWeaponItemDefinition* SmallShield = LoadObject<UFortWeaponItemDefinition>(nullptr,
		TEXT("/Game/Athena/Items/Consumables/ShieldSmall/Athena_ShieldSmall.Athena_ShieldSmall"));
	UFortWeaponItemDefinition* LargeShield = LoadObject<UFortWeaponItemDefinition>(nullptr,
		TEXT("/Game/Athena/Items/Consumables/Shields/Athena_Shields.Athena_Shields"));
	const UClass* ShieldAbilityClass = SmallShield ? SmallShield->GetPrimaryFireAbility() : nullptr;
	TestNotNull(TEXT("Cooked small shield item"), SmallShield);
	TestNotNull(TEXT("Cooked small shield primary ability"), ShieldAbilityClass);
	TestNotNull(TEXT("Cooked large shield item"), LargeShield);
	const UClass* LargeShieldAbilityClass = LargeShield ? LargeShield->GetPrimaryFireAbility() : nullptr;
	TestNotNull(TEXT("Cooked large shield primary ability"), LargeShieldAbilityClass);

	bCleanupQueued = true;
	const TSharedRef<FGuid> ShieldGuid = MakeShared<FGuid>();
	const TWeakObjectPtr<UFortWeaponItemDefinition> WeakSmallShield(SmallShield);
	const TWeakObjectPtr<UFortWeaponItemDefinition> WeakLargeShield(LargeShield);
	const TWeakObjectPtr<AFortPlayerPawnAthena> WeakPawn(Pawn);
	const TWeakObjectPtr<AFortWeapon> WeakEquipped(Equipped);
	const TWeakObjectPtr<AFortPlayerControllerAthena> WeakController(Controller);
	const TWeakObjectPtr<AFortInventory> WeakInventory(Inventory);
	const auto ActivateShield = [this, WeakPawn, WeakEquipped, WeakController, ShieldGuid](UFortWeaponItemDefinition* Shield, const TCHAR* Name)
	{
		AFortPlayerPawnAthena* LivePawn = WeakPawn.Get();
		AFortWeapon* LiveWeapon = WeakEquipped.Get();
		AFortPlayerControllerAthena* LiveController = WeakController.Get();
		if (!TestNotNull(TEXT("Consumable pawn for shield use"), LivePawn) ||
			!TestNotNull(TEXT("Consumable weapon for shield use"), LiveWeapon) ||
			!TestNotNull(TEXT("Consumable controller for shield use"), LiveController) ||
			!TestNotNull(*FString::Printf(TEXT("%s shield definition"), Name), Shield))
		{
			return;
		}
		UFortWorldItem* ShieldItem = FortAthenaInventory::GiveItem(LiveController, Shield, 2);
		UClass* ShieldClass = Shield->GetPrimaryFireAbility();
		if (!TestNotNull(*FString::Printf(TEXT("%s shield stack"), Name), ShieldItem) ||
			!TestNotNull(*FString::Printf(TEXT("%s shield ability class"), Name), ShieldClass))
		{
			return;
		}
		*ShieldGuid = ShieldItem->NativeGetItemGuid();
		LivePawn->SetMaxShield(100.0f);
		LivePawn->SetShield(0.0f);
		LiveWeapon->WeaponData = Shield;
		LiveWeapon->ItemEntryGuid = *ShieldGuid;
		LivePawn->CurrentWeapon = LiveWeapon;
		LivePawn->CurrentWeaponList.AddUnique(LiveWeapon);
		UAbilitySystemComponent* LiveASC = LivePawn->GetAbilitySystemComponent();
		if (!TestNotNull(TEXT("Shield ability system"), LiveASC))
		{
			return;
		}
		const FGameplayAbilitySpecHandle ShieldHandle = LiveASC->GiveAbility(
			FGameplayAbilitySpec(ShieldClass, 1, INDEX_NONE, LiveWeapon));
		if (TestTrue(*FString::Printf(TEXT("Cooked %s shield ability is granted"), Name), ShieldHandle.IsValid()))
		{
			TGuardValue<TWeakObjectPtr<APlayerController>> LocalActivation(
				LiveASC->AbilityActorInfo->PlayerController, TWeakObjectPtr<APlayerController>());
			TestTrue(*FString::Printf(TEXT("Equipped %s shield ability activates"), Name), LiveASC->TryActivateAbility(ShieldHandle, false));
		}
	};
	const auto CheckShield = [this, WeakPawn, WeakInventory, ShieldGuid](const TCHAR* Name, float ExpectedShield)
	{
		if (TestNotNull(TEXT("Consumable pawn after shield use"), WeakPawn.Get()))
		{
			TestEqual(*FString::Printf(TEXT("Using a %s shield restores shield"), Name), WeakPawn->GetShield(), ExpectedShield);
		}
		if (TestNotNull(TEXT("Consumable inventory after shield use"), WeakInventory.Get()))
		{
			UFortWorldItem* RemainingShield = WeakInventory->NativeFindItemByGuid(*ShieldGuid);
			if (TestNotNull(*FString::Printf(TEXT("%s shield stack remains after use"), Name), RemainingShield))
			{
				TestEqual(*FString::Printf(TEXT("Using a %s shield spends one unit"), Name), RemainingShield->ItemEntry.Count, 1);
			}
		}
	};
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand(([this, WeakPawn, WeakInventory, WeakSmallShield, LastBandageGuid, ActivateShield]()
	{
		if (TestNotNull(TEXT("Consumable pawn after bandage use"), WeakPawn.Get()))
		{
			TestEqual(TEXT("Using the bandage restores the expected health"), WeakPawn->GetHealth(), 65.0f);
		}
		if (TestNotNull(TEXT("Consumable inventory after bandage use"), WeakInventory.Get()))
		{
			TestNull(TEXT("Successful use removes the last bandage"), WeakInventory->NativeFindItemByGuid(LastBandageGuid));
		}
		ActivateShield(WeakSmallShield.Get(), TEXT("small"));
	}), 8.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand(([WeakLargeShield, ActivateShield, CheckShield]()
	{
		CheckShield(TEXT("small"), 25.0f);
		ActivateShield(WeakLargeShield.Get(), TEXT("large"));
	}), 8.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand(([WeakPawn, WeakEquipped, WeakController, WeakInventory, CheckShield]()
	{
		CheckShield(TEXT("large"), 50.0f);
		if (AFortPlayerPawnAthena* LivePawn = WeakPawn.Get())
		{
			LivePawn->Destroy();
		}
		if (AFortWeapon* LiveWeapon = WeakEquipped.Get())
		{
			LiveWeapon->Destroy();
		}
		if (AFortPlayerControllerAthena* LiveController = WeakController.Get())
		{
			LiveController->WorldInventory = nullptr;
			LiveController->Destroy();
		}
		if (AFortInventory* LiveInventory = WeakInventory.Get())
		{
			LiveInventory->Destroy();
		}
	}), 8.0f));
	return true;
}

#endif
