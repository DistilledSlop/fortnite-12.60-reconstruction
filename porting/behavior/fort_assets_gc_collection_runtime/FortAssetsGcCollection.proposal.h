#pragma once

#include "CoreMinimal.h"
#include "UObject/UObjectGlobals.h"

// Proposal for FFortAssets +0x150 only. The value records a source FName.
// The owner supplies lifetime and synchronization, including the observed
// traversal outside its main critical section.
struct FFortAssetsGcCollection
{
    TMap<UObject*, FName> Entries;

    // 0x141de9f00: null keys are rejected; an existing key's value is replaced.
    void Add(UObject* Object, FName SourceName)
    {
        if (Object)
        {
            Entries.Add(Object, SourceName);
        }
    }

    // 0x141de3560: the map overload visits keys only for this value type.
    void AddReferencedObjects(FReferenceCollector& Collector)
    {
        Collector.AddReferencedObjects(Entries);
    }

    // 0x140992250 with Slack == 0, called under the owner's lock.
    void Empty()
    {
        Entries.Empty(0);
    }
};
