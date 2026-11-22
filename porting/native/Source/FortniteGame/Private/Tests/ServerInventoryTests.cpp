#if WITH_DEV_AUTOMATION_TESTS

#include "AbilitySystemComponent.h"
#include "AFortInventory.h"
#include "AFortWeapon.h"
#include "AFortPlayerControllerAthena.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "FortAthenaInventoryFlow.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "UFortGameplayAbility.h"
#include "UFortResourceItemDefinition.h"
#include "UFortWeaponItemDefinition.h"
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
	UFortWorldItem* Item = FortAthenaInventory::GiveItem(Controller, Bandage, 1);
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
	TestTrue(TEXT("Commit can spend the last bandage"), AbilityCDO->CommitAbilityCost(Handle, &ActorInfo, ActivationInfo));
	TestNull(TEXT("Depleted source stack is removed"), Inventory->NativeFindItemByGuid(Guid));
	TestFalse(TEXT("Commit fails after the source stack is gone"), AbilityCDO->CommitAbilityCost(Handle, &ActorInfo, ActivationInfo));
	return true;
}

#endif
