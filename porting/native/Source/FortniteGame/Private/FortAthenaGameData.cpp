#include "FortAthenaGameData.h"

#include "UFortAssetManager.h"
#include "UFortGameData.h"
#include "Engine/Engine.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UObject/UObjectGlobals.h"

void FortAthenaGameData::Initialize()
{
    if (!FParse::Param(FCommandLine::Get(), TEXT("FortniteOriginalClient"))) return;
    UFortAssetManager* Manager = GEngine ? Cast<UFortAssetManager>(GEngine->AssetManager) : nullptr;
    if (!Manager)
    {
        UE_LOG(LogTemp, Error, TEXT("GameData: configured AssetManager is not FortAssetManager"));
        return;
    }
    // Reconstruction glue: resolve the original configured asset directly until
    // the original primary-asset scanning/startup sequence is recovered. Keep the
    // cache on the manager's UPROPERTY, matching its existing common-data getter.
    if (!Manager->GameDataCommon && !Manager->GameDataNameCommon.IsEmpty())
        Manager->GameDataCommon = LoadObject<UFortGameData>(Manager, *Manager->GameDataNameCommon);
    if (!Manager->GameDataCommon)
    {
        UE_LOG(LogTemp, Error, TEXT("GameData: common data failed to load: %s"), *Manager->GameDataNameCommon);
        return;
    }
    UE_LOG(LogTemp, Display, TEXT("GameData: manager=%s common=%s lootLevel=%s deathMapping=%s"),
        *Manager->GetClass()->GetPathName(), *Manager->GameDataCommon->GetPathName(),
        *Manager->GameDataCommon->LootLevelData.ToSoftObjectPath().ToString(),
        *Manager->GameDataCommon->FortDeathCauseFromTagMapping.ToSoftObjectPath().ToString());
}
