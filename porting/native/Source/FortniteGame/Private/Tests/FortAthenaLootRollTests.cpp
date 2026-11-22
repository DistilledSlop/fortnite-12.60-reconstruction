// Optional integration: Private/Tests/FortAthenaLootRollTests.cpp.
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "FortAthenaLootRoll.h"
#include "FFortLootTierData.h"
#include "FFortLootPackageData.h"
#include "UFortItemDefinition.h"
#include "ABuildingContainer.h"
#include "AFortGameModeAthena.h"
#include "AFortPickupAthena.h"
#include "AFortPlayerControllerAthena.h"
#include "AFortPlayerPawnAthena.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "FortAthenaLootData.h"
#include "FortAthenaLootSearch.h"
#include "FortAthenaLootWorld.h"
#include "Misc/ScopeExit.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFortLootContractTest, "Fortnite.Server.Loot.ReconstructedContract",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FFortLootContractTest::RunTest(const FString&)
{
    using namespace FortAthenaLootRoll;
    FRequest Q; Q.Group = FName(TEXT("test")); Q.LootLevel = 7;
    FFortLootTierData Tier;
    Tier.Weight = 1; Tier.LootPackage = FName(TEXT("root")); Tier.NumLootPackageDrops = 1;
    Tier.LootPackageCategoryWeightArray = {1}; Tier.LootPackageCategoryMinArray = {1}; Tier.LootPackageCategoryMaxArray = {-1};
    FFortLootPackageData A, B, Nested;
    A.LootPackageID = B.LootPackageID = FName(TEXT("root"));
    A.Weight = 1; B.Weight = 3; A.Count = 2; B.Count = 5;
    A.LootPackageCategory = B.LootPackageCategory = 0;
    A.ItemDefinition = B.ItemDefinition = GetMutableDefault<UFortItemDefinition>();
    TArray<const FFortLootPackageData*> Packages = {&A, &B};
    TArray<double> Random; int32 Cursor = 0;
    FServices S;
    S.ResolveGroup = [](FName In, FName& Out) { Out = In; return true; };
    S.FindTiers = [&Tier](FName, TArray<const FFortLootTierData*>& Out) { Out.Add(&Tier); };
    S.FindPackages = [&Packages](FName Id, TArray<const FFortLootPackageData*>& Out)
        { for (auto* P : Packages) if (P->LootPackageID == Id) Out.Add(P); };
    S.Random01 = [&]() { return Random.IsValidIndex(Cursor) ? Random[Cursor++] : 0.0; };
    S.ResolveItemLevel = [](UFortItemDefinition*, int32 In, int32& Out) { Out = In + 1; return true; };
    TArray<FDrop> Drops; FString Error;
    auto Run = [&](std::initializer_list<double> Values)
    { Random.Reset(); for (double V : Values) Random.Add(V); Cursor = 0; return Roll(Q, S, Drops, Error); };
    TestTrue(TEXT("RNG zero picks first positive row"), Run({0, 0}));
    TestEqual(TEXT("first row count preserved"), Drops.Num() ? Drops[0].Count : -1, 2);
    TestEqual(TEXT("resolver level preserved"), Drops.Num() ? Drops[0].Level : -1, 8);
    TestTrue(TEXT("exact weighted boundary picks next row"), Run({0, .25}));
    TestEqual(TEXT("boundary selects second count"), Drops.Num() ? Drops[0].Count : -1, 5);
    TestTrue(TEXT("near one picks final positive row"), Run({0, .999999999}));
    TestEqual(TEXT("near one second count"), Drops.Num() ? Drops[0].Count : -1, 5);
    Tier.NumLootPackageDrops = 1.6f;
    TestTrue(TEXT("fraction lower side adds drop"), Run({0, .5, 0, 0, 0}));
    TestEqual(TEXT("fraction lower result"), Drops.Num(), 2);
    TestTrue(TEXT("fraction upper side keeps floor"), Run({0, .9, 0}));
    TestEqual(TEXT("fraction upper result"), Drops.Num(), 1);
    Tier.NumLootPackageDrops = 0; Tier.LootPackageCategoryMinArray[0] = 2;
    TestTrue(TEXT("minima override lower target"), Run({0, 0, 0}));
    TestEqual(TEXT("both mandatory drops"), Drops.Num(), 2);
    Tier.NumLootPackageDrops = 3; Tier.LootPackageCategoryMaxArray[0] = 2;
    TestFalse(TEXT("max exhaustion fails"), Run({0, 0, 0}));
    TestEqual(TEXT("exhaustion returns no partial drops"), Drops.Num(), 0);
    Tier.NumLootPackageDrops = 1; Tier.LootPackageCategoryMinArray[0] = 1; Tier.LootPackageCategoryMaxArray[0] = -1;
    A.ItemDefinition.Reset(); A.LootPackageCall = TEXT("child"); A.Count = 3; B.Weight = 0;
    Nested = B; Nested.Weight = 1; Nested.Count = 4; Nested.LootPackageID = FName(TEXT("child")); Packages.Add(&Nested);
    TestTrue(TEXT("call Count repeats child draws"), Run({0, 0, 0, 0, 0}));
    TestEqual(TEXT("three child selections"), Drops.Num(), 3);
    TestEqual(TEXT("terminal count is not call count"), Drops.Num() ? Drops[0].Count : -1, 4);
    Nested.ItemDefinition.Reset(); Nested.LootPackageCall = TEXT("root");
    TestFalse(TEXT("cycle is explicit failure"), Run({0, 0, 0}));
    TestTrue(TEXT("cycle reported"), Error.Contains(TEXT("cycle")));
    TestEqual(TEXT("cycle atomic output"), Drops.Num(), 0);
    A.Weight = 0;
    TestFalse(TEXT("zero positive candidates fails"), Run({0}));
    A.Weight = 1; A.NamedWeightMult = FName(TEXT("unknown"));
    TestFalse(TEXT("unsupported named weights fails"), Run({0}));
    TestTrue(TEXT("named error reported"), Error.Contains(TEXT("named")));
    A.NamedWeightMult = NAME_None;
    TestFalse(TEXT("RNG upper endpoint rejected"), Run({1}));
    return true;
}
// Exercises the production roll and search path with the dedicated server's initialized loot data.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFortContainerSearchTransactionTest, "Fortnite.Server.Loot.ContainerSearchTransaction",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)

bool FFortContainerSearchTransactionTest::RunTest(const FString&)
{
    if (!IsRunningDedicatedServer() || !FortAthenaLootSearch::IsEnabled())
        return true;

    UWorld* World = nullptr;
    for (const FWorldContext& Context : GEngine->GetWorldContexts())
        if (Context.World() && Context.World()->GetNetMode() == NM_DedicatedServer)
        {
            World = Context.World();
            break;
        }
    if (!TestNotNull(TEXT("Dedicated server world"), World)) return false;
    AFortGameModeAthena* Mode = World->GetAuthGameMode<AFortGameModeAthena>();
    if (!TestNotNull(TEXT("Athena game mode"), Mode) ||
        !TestTrue(TEXT("Loot data is initialized"), FortAthenaLootData::IsReady(Mode))) return false;

    AFortPlayerControllerAthena* Controller = World->SpawnActor<AFortPlayerControllerAthena>();
    AFortPlayerPawnAthena* Pawn = World->SpawnActor<AFortPlayerPawnAthena>(FVector::ZeroVector, FRotator::ZeroRotator);
    ABuildingContainer* Chest = World->SpawnActor<ABuildingContainer>();
    ABuildingContainer* Ammo = World->SpawnActor<ABuildingContainer>();
    ABuildingContainer* FloorMarker = nullptr;
    TArray<AFortPickupAthena*> Pickups;
    ON_SCOPE_EXIT
    {
        for (AFortPickupAthena* Pickup : Pickups) if (IsValid(Pickup)) Pickup->Destroy();
        if (IsValid(Chest)) Chest->Destroy();
        if (IsValid(Ammo)) Ammo->Destroy();
        if (IsValid(FloorMarker)) FloorMarker->Destroy();
        if (IsValid(Controller)) Controller->Destroy();
        if (IsValid(Pawn)) Pawn->Destroy();
    };
    if (!TestNotNull(TEXT("Controller"), Controller) || !TestNotNull(TEXT("Pawn"), Pawn) ||
        !TestNotNull(TEXT("Chest"), Chest) || !TestNotNull(TEXT("Ammo container"), Ammo)) return false;
    Controller->Possess(Pawn);
    if (!TestTrue(TEXT("Controller possesses pawn"), Pawn->GetController() == Controller)) return false;
    Chest->SearchLootTierGroup = FName(TEXT("Loot_AthenaTreasure"));
    Chest->bDestroyContainerOnSearch = false;
    Ammo->SearchLootTierGroup = FName(TEXT("Loot_AthenaAmmoLarge"));
    Ammo->bDestroyContainerOnSearch = false;

    int32 FailSpawnNumber = 2;
    int32 SpawnCount = 0;
    const FDelegateHandle SpawnCallback = World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateLambda([&](AActor* Actor)
    {
        if (AFortPickupAthena* Pickup = Cast<AFortPickupAthena>(Actor))
        {
            Pickups.Add(Pickup);
            ++SpawnCount;
            if (SpawnCount == FailSpawnNumber) Pickup->Destroy();
        }
    }));
    ON_SCOPE_EXIT { World->RemoveOnActorSpawnedHandler(SpawnCallback); };

    TestFalse(TEXT("A failed second pickup aborts the chest search"), FortAthenaLootSearch::TrySearchContainer(Controller, Chest));
    TestEqual(TEXT("Failure was injected on the second pickup"), SpawnCount, 2);
    TestFalse(TEXT("Failed chest remains unsearched"), Chest->bAlreadySearched != 0);
    for (AFortPickupAthena* Pickup : Pickups)
        TestFalse(TEXT("Failed search leaves no live pickup"), IsValid(Pickup) && !Pickup->IsActorBeingDestroyed());

    FailSpawnNumber = 0;
    const int32 BeforeRetry = SpawnCount;
    TestTrue(TEXT("Chest search can be retried"), FortAthenaLootSearch::TrySearchContainer(Controller, Chest));
    TestTrue(TEXT("Successful chest is marked searched"), Chest->bAlreadySearched != 0);
    TestTrue(TEXT("Retry creates pickups"), SpawnCount > BeforeRetry);
    for (int32 Index = BeforeRetry; Index < Pickups.Num(); ++Index)
        if (TestTrue(TEXT("Retry pickup is live"), IsValid(Pickups[Index])))
            TestTrue(TEXT("Retry pickup has a populated item"), Pickups[Index]->PrimaryPickupItemEntry.ItemDefinition && Pickups[Index]->PrimaryPickupItemEntry.Count > 0);
    const int32 AfterChest = SpawnCount;
    TestFalse(TEXT("Searched chest cannot be searched again"), FortAthenaLootSearch::TrySearchContainer(Controller, Chest));
    TestEqual(TEXT("Repeated chest search creates no pickups"), SpawnCount, AfterChest);

    TestTrue(TEXT("Ammo container search succeeds"), FortAthenaLootSearch::TrySearchContainer(Controller, Ammo));
    TestTrue(TEXT("Ammo container is marked searched"), Ammo->bAlreadySearched != 0);
    TestTrue(TEXT("Ammo search creates pickups"), SpawnCount > AfterChest);
    for (int32 Index = AfterChest; Index < Pickups.Num(); ++Index)
        if (TestTrue(TEXT("Ammo pickup is live"), IsValid(Pickups[Index])))
            TestTrue(TEXT("Ammo pickup has a populated item"), Pickups[Index]->PrimaryPickupItemEntry.ItemDefinition && Pickups[Index]->PrimaryPickupItemEntry.Count > 0);
    const int32 AfterAmmo = SpawnCount;
    TestFalse(TEXT("Searched ammo container cannot be searched again"), FortAthenaLootSearch::TrySearchContainer(Controller, Ammo));
    TestEqual(TEXT("Repeated ammo search creates no pickups"), SpawnCount, AfterAmmo);
    FloorMarker = World->SpawnActor<ABuildingContainer>();
    if (!TestNotNull(TEXT("Floor loot marker"), FloorMarker)) return false;
    FloorMarker->SearchLootTierGroup = Chest->SearchLootTierGroup;
    FloorMarker->bStartAlreadySearched_Athena = true;
    FloorMarker->bDestroyContainerOnSearch = false;

    FailSpawnNumber = SpawnCount + 2;
    FortAthenaLootWorld::PopulateLoadedMarkers(Mode, FloorMarker->GetLevel());
    TestEqual(TEXT("Floor loot failure was injected"), SpawnCount, FailSpawnNumber);
    TestFalse(TEXT("Failed floor loot marker remains unsearched"), FloorMarker->bAlreadySearched != 0);
    for (int32 Index = AfterAmmo; Index < Pickups.Num(); ++Index)
        TestFalse(TEXT("Failed floor loot leaves no live pickup"), IsValid(Pickups[Index]));

    const int32 BeforeFloorRetry = SpawnCount;
    FortAthenaLootWorld::Tick(Mode);
    TestTrue(TEXT("Floor loot retry populates the marker"), FloorMarker->bAlreadySearched != 0);
    TestTrue(TEXT("Floor loot retry creates pickups"), SpawnCount > BeforeFloorRetry);
    const int32 AfterFloorRetry = SpawnCount;
    FortAthenaLootWorld::Tick(Mode);
    TestEqual(TEXT("Successful floor loot is not spawned twice"), SpawnCount, AfterFloorRetry);
    return true;
}
#endif
