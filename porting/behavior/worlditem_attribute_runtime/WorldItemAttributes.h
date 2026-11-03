#pragma once

#include "DeserializeAttributes.h"
#include "Engine/Engine.h"
#include "FFortItemEntry.h"
#include "SharedFortGameLog.h"
#include "UFortAlterationItemDefinition.h"
#include "UFortAssetManager.h"

// Proposed equivalent of 0x141df62a0. The fatal branch's allocation is the
// native fallback; the fatal log normally terminates before it is reached.
inline UFortAssetManager& GetFortAssetManagerForAttributes()
{
    UFortAssetManager* Manager = GEngine ? Cast<UFortAssetManager>(GEngine->AssetManager) : nullptr;
    if (!Manager)
    {
        UE_LOG(LogFort, Fatal, TEXT("Invalid AssetManager class used, must be UFortAssetManager"));
        Manager = NewObject<UFortAssetManager>();
    }
    return *Manager;
}

// Proposed equivalent of 0x141df6530. The original uses FString::Split, so
// retain that behavior, including an invalid ID when there is no colon.
inline UObject* FindFortAssetByPrimaryId(UFortAssetManager& Manager, const FString& IdString)
{
    FString Type;
    FString Name;
    FPrimaryAssetId Id;
    if (IdString.Split(TEXT(":"), &Type, &Name))
    {
        Id = FPrimaryAssetId(FPrimaryAssetType(FName(*Type)), FName(*Name));
    }
    UObject* Asset = Manager.GetPrimaryAssetObject(Id);
    return Asset ? Asset : Manager.GetPrimaryAssetPath(Id).TryLoad();
}

// Proposed equivalent of 0x142035080.
inline bool PopulateWorldItemAttributes(
    FFortItemEntry& Entry,
    const TMap<FString, TSharedPtr<FJsonValue>>& Attributes)
{
    if (!DeserializeMcpAttributes(Attributes, &Entry, FFortItemEntry::StaticStruct()))
    {
        return false;
    }

    Entry.AlterationInstances.Reset(Entry.AlterationDefinitions.Num());
    for (const FString& Definition : Entry.AlterationDefinitions)
    {
        UObject* Asset = FindFortAssetByPrimaryId(GetFortAssetManagerForAttributes(), Definition);
        if (UFortAlterationItemDefinition* Alteration = Cast<UFortAlterationItemDefinition>(Asset))
        {
            Entry.AlterationInstances.Add(Alteration);
        }
        else
        {
            UE_LOG(LogFort, Warning, TEXT("Failed to find asset for alteration %s"), *Definition);
        }
    }
    return true;
}
