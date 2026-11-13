#pragma once

#include "CoreMinimal.h"

class AFortGameModeAthena;
class UFortPlaylistAthena;
class UDataTable;
struct FFortLootTierData;
struct FFortLootPackageData;

// Server reconstruction glue. Row pointers remain valid only until world cleanup or reinitialization.
namespace FortAthenaLootData
{
    bool Initialize(AFortGameModeAthena* Mode, UFortPlaylistAthena* Playlist);
    bool IsReady(const AFortGameModeAthena* Mode);
    bool ResolveGroup(const AFortGameModeAthena* Mode, FName Input, FName& Output);
    void FindTiers(const AFortGameModeAthena* Mode, FName Group, TArray<const FFortLootTierData*>& Out);
    void FindPackages(const AFortGameModeAthena* Mode, FName Id, TArray<const FFortLootPackageData*>& Out);
    int32 GetLootLevel(const AFortGameModeAthena* Mode);
    const UDataTable* GetLootLevelTable(const AFortGameModeAthena* Mode);
}
