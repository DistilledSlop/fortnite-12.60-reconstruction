#pragma once
#include "CoreMinimal.h"
class AFortGameModeAthena;
class UFortItemDefinition;
struct FFortLootTierData;
struct FFortLootPackageData;

// Reconstructed ordinary-loot contract; original server parity is unresolved.
namespace FortAthenaLootRoll
{
    struct FRequest
    {
        FName Group;
        int32 WorldLevel = 0;
        int32 LootTier = -1; // -1 permits all tiers; other values match exactly.
        int32 LootLevel = 0;
        bool bRequestBonus = false; // Unsupported until bonus trigger semantics are recovered.
        int32 MaxDepth = 64;
        int32 MaxDecisions = 4096;
    };
    struct FDrop
    {
        UFortItemDefinition* Definition = nullptr;
        int32 Count = 0;
        int32 Level = 0;
    };
    // Injected providers make table/RNG/error cases independently testable.
    // Level resolution must implement the caller's explicit item-level contract.
    struct FServices
    {
        TFunction<bool(FName, FName&)> ResolveGroup;
        TFunction<void(FName, TArray<const FFortLootTierData*>&)> FindTiers;
        TFunction<void(FName, TArray<const FFortLootPackageData*>&)> FindPackages;
        TFunction<double()> Random01; // finite, [0,1); one draw per weighted choice.
        TFunction<bool(UFortItemDefinition*, int32, int32&)> ResolveItemLevel;
    };
    bool Roll(const FRequest& Request, const FServices& Services, TArray<FDrop>& Out, FString& Error);
    bool RollForMode(const AFortGameModeAthena* Mode, const FRequest& Request,
        TFunction<double()> Random01,
        TFunction<bool(UFortItemDefinition*, int32, int32&)> ResolveItemLevel,
        TArray<FDrop>& Out, FString& Error);
}
