// Optional integration: Private/Tests/FortAthenaLootRollTests.cpp.
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "FortAthenaLootRoll.h"
#include "FFortLootTierData.h"
#include "FFortLootPackageData.h"
#include "UFortItemDefinition.h"

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
#endif
