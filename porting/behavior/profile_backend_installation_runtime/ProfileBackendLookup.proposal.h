#pragma once

#include "CoreMinimal.h"

// Logical implementation proposal, not an assertion about the original class layout.
// TEntryAccess::Id(const TEntry&) returns the FString at original entry +0x10;
// TEntryAccess::Quantity(const TEntry&) returns int32 at +0x30. The canonical
// entry must retain the UObject at +0x88 through the profile's GC collector.
// TOverlay::FindExact(Id) returns a pointer to the existing entry shared pointer,
// or nullptr; original 0x1410fa050 compares Id case-sensitively. Its underlying
// key is a shared string, so the adapter must preserve that comparison behavior.
template <typename TEntry, typename TEntryAccess, typename TOverlay>
class TProfileBackendLookupProposal
{
public:
    using FEntryPtr = TSharedPtr<TEntry, ESPMode::NotThreadSafe>;
    using FEntries = TArray<FEntryPtr>;
    using FEntryMap = TMap<FString, FEntries>;

    // The caller must maintain these collections through the real profile update
    // paths and mark affected definition keys dirty when overlays change.
    FEntryMap BaseByDefinition;
    TOverlay OverridesByEntryId;
    FEntryMap CachedByDefinition;
    TSet<FString> DirtyDefinitions;

    const FEntries* FindBase(const FString& DefinitionKey) const
    {
        return BaseByDefinition.Find(DefinitionKey);
    }

    const FEntries* FindWithOverrides(const FString& DefinitionKey)
    {
        if (DirtyDefinitions.Remove(DefinitionKey) == 0)
        {
            if (const FEntries* Cached = CachedByDefinition.Find(DefinitionKey))
            {
                return Cached;
            }
            return FindBase(DefinitionKey);
        }

        FEntries Combined;
        int32 OverrideMatches = 0;
        if (const FEntries* Base = FindBase(DefinitionKey))
        {
            for (const FEntryPtr& Entry : *Base)
            {
                // Original assumes every base entry pointer is valid.
                const FEntryPtr* Override = OverridesByEntryId.FindExact(TEntryAccess::Id(*Entry));
                if (!Override)
                {
                    Combined.Add(Entry);
                }
                else
                {
                    ++OverrideMatches;
                    if (TEntryAccess::Quantity(**Override) > 0)
                    {
                        Combined.Add(*Override);
                    }
                }
            }
        }
        if (OverrideMatches == 0)
        {
            CachedByDefinition.Remove(DefinitionKey);
            return FindBase(DefinitionKey);
        }
        FEntries& Cached = CachedByDefinition.FindOrAdd(DefinitionKey);
        Cached = MoveTemp(Combined);
        return &Cached;
    }
};
