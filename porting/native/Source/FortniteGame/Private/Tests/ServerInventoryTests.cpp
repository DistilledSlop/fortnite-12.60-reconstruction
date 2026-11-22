#if WITH_DEV_AUTOMATION_TESTS

#include "ABuildingWall.h"
#include "AbilitySystemComponent.h"
#include "AFortInventory.h"
#include "AFortPickupAthena.h"
#include "AFortPlayerPawnAthena.h"
#include "AFortWeapon.h"
#include "AFortPlayerControllerAthena.h"
#include "AFortPlayerStateAthena.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "EWorldItemDropBehavior.h"
#include "FortAthenaBuildDamage.h"
#include "FortAthenaInventoryFlow.h"
#include "FortAthenaLootTransfer.h"
#include "FortHarvestBuilding.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "UFortGameplayAbility.h"
#include "UFortKismetLibrary.h"
#include "UFortResourceItemDefinition.h"
#include "UFortWeaponItemDefinition.h"
#include "UFortWeaponRangedItemDefinition.h"
#include "UFortWorldItem.h"

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
	Definition->MaxStackSize = FScalableFloat(10.0f);
	Definition->bAllowMultipleStacks = true;

	int32 Accepted = 0;
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
	AFortPlayerPawnAthena* Pawn = World->SpawnActor<AFortPlayerPawnAthena>(FVector(0, 0, 16000), FRotator::ZeroRotator);
	if (!TestNotNull(TEXT("Inventory transfer pawn"), Pawn))
	{
		return false;
	}
	ON_SCOPE_EXIT { Pawn->Destroy(); };
	Controller->Possess(Pawn);
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

	TestTrue(TEXT("Drop splits four units from the stack"), FortAthenaLootTransfer::AttemptInventoryDrop(Controller, SecondGuid, 4, false));
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
	TestFalse(TEXT("A second player cannot collect a claimed pickup"),
		FortAthenaLootTransfer::HandlePickup(ContenderPawn, Pickup, 0.25f, FVector::ZeroVector, false));
	TestEqual(TEXT("The losing contender receives no replicated items"), ContenderInventory->Inventory.ReplicatedEntries.Num(), 0);
	if (!Dropped || !TestTrue(TEXT("The second player can collect the split drop"),
		FortAthenaLootTransfer::HandlePickup(ContenderPawn, Dropped, 0.25f, FVector::ZeroVector, false)))
	{
		return false;
	}
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
	TestEqual(TEXT("Elimination drops only the eligible weapon"), FortAthenaLootTransfer::DropAllOnDeath(Controller, Pawn), 1);
	TestNull(TEXT("Elimination removes the dropped weapon from inventory"), Inventory->NativeFindItemByGuid(RecoveredGuid));
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
		TestTrue(TEXT("Elimination loot contains no protected item"), It->PrimaryPickupItemEntry.ItemDefinition == WeaponDefinition);
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
	if (!TestTrue(TEXT("The damaged wall has a material cost"), RepairCost > 0))
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
	UFortWorldItem* RepairStack = FortAthenaInventory::GiveItem(Controller, RepairMaterial, RepairCost + 1);
	if (!TestNotNull(TEXT("Repair material stack"), RepairStack))
	{
		return false;
	}
	TestTrue(TEXT("Repair starts after the material debit"), FortAthenaBuildDamage::RepairBuildingActor(Controller, Wall));
	TestEqual(TEXT("Repair removes exactly its cost"), RepairStack->ItemEntry.Count, 1);
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
	if (!TestNotNull(TEXT("A second large stack of the same item can be granted"),
		FortAthenaInventory::GiveItem(Controller, BundleFirst, BundleStackCount)))
	{
		return false;
	}
	UFortKismetLibrary* Library = GetMutableDefault<UFortKismetLibrary>();
	UFunction* QueryFunction = Library->FindFunction(FName(TEXT("K2_GetItemQuantityOnPlayer")));
	UFunction* RemoveFunction = Library->FindFunction(FName(TEXT("K2_RemoveItemFromPlayer")));
	if (!TestNotNull(TEXT("Blueprint quantity function"), QueryFunction) ||
		!TestNotNull(TEXT("Blueprint removal function"), RemoveFunction))
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
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FServerConsumableCostDataTest, "Fortnite.Server.Inventory.ConsumableCostData",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)

bool FServerConsumableCostDataTest::RunTest(const FString&)
{
	// The headless stage omits audiovisual packages referenced by these cooked assets.
	AddExpectedError(TEXT("Couldn't find file for package (/Game/Athena/Sounds/Items/Item_Consumables_(Bandage|Medkit)_Apply_Cue|/Game/Effects/Fort_Effects/Effects/Characters/Generic/Consumables/P_Consumable_(ItemUseComplete|MedKit_PourDecal)|/Game/Packages/Fortress_SharedMaterials/Base_Material/DEMO_Master|/Game/Sounds/Fort_GamePlay_Sounds/Loot/EatHealth_Restore_Athena_Cue) "), EAutomationExpectedErrorFlags::Contains, 0);
	AddExpectedError(TEXT("Found 1 dependent packages"), EAutomationExpectedErrorFlags::Contains, 0);
	AddExpectedError(TEXT("  /Game/(Animation/Game/MainPlayer/Montages/General/(ApplyMedkit|WrapBandages)_Athena|Athena/Items/Consumables/Bandage/BandageCuff_Bandage_Roll_LOD0)$"), EAutomationExpectedErrorFlags::Contains, 0);
	AddExpectedError(TEXT("Attempt to build nav collision data for /Game/Athena/Items/Consumables/Bandage/BandageCuff_Bandage_Roll_LOD0"), EAutomationExpectedErrorFlags::Contains, 0);

	UFortWeaponItemDefinition* Bandage = LoadObject<UFortWeaponItemDefinition>(nullptr,
		TEXT("/Game/Athena/Items/Consumables/Bandage/Athena_Bandage.Athena_Bandage"));
	if (!TestNotNull(TEXT("Cooked bandage item"), Bandage))
	{
		return false;
	}
	TestEqual(TEXT("Bandage stack cap resolves from game data"), Bandage->NativeGetMaxStackSize(nullptr), 15);
	TestTrue(TEXT("Bandage uses its inventory definition as ammo"), Bandage->GetAmmoWorldItemDefinitionNative() == Bandage);

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
	ON_SCOPE_EXIT
	{
		if (Weapon)
		{
			Weapon->Destroy();
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
	UFortWorldItem* Item = FortAthenaInventory::GiveItem(Controller, Bandage, 2);
	if (!TestNotNull(TEXT("Bandage stack"), Item))
	{
		return false;
	}
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
	TestTrue(TEXT("Commit can spend the last bandage"), AbilityCDO->CommitAbilityCost(Handle, &ActorInfo, ActivationInfo));
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
	return true;
}

#endif
