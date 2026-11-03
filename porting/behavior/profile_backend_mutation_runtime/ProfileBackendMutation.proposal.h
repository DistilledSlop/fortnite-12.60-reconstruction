#pragma once

#include "CoreMinimal.h"

// Logical evidence-gated building blocks. The backing owns all containers;
// these functions do not claim the binary layout of the captured backing.
// CompareItemId must implement 0x142bcf780 (ASCII-folding, otherwise exact
// UTF-16 code units), consistently with the native FString map key functions.
template <typename TEntry, typename TAccess, typename TCompareItemId>
struct TProfileBackendMutationProposal
{
    using FEntryPtr = TSharedPtr<TEntry, ESPMode::NotThreadSafe>;
    using FEntries = TArray<FEntryPtr>;
    using FEntryMap = TMap<FString, FEntries>;

    static void InsertGroup(FEntryMap& Groups, const FString& Key, const FEntryPtr& Entry)
    {
        FEntries& Group = Groups.FindOrAdd(Key);
        int32 First = 0;
        int32 Count = Group.Num();
        while (Count > 0)
        {
            const int32 Half = Count / 2;
            const int32 Mid = First + Half;
            if (TCompareItemId::Compare(TAccess::ItemId(*Group[Mid]), TAccess::ItemId(*Entry)) < 0)
            {
                First = Mid + 1;
                Count -= Half + 1;
            }
            else
            {
                Count = Half;
            }
        }
        if (First == Group.Num() || TCompareItemId::Compare(
                TAccess::ItemId(*Group[First]), TAccess::ItemId(*Entry)) != 0)
        {
            Group.Insert(Entry, First);
        }
    }

    // Caller must insert into the ID map through its recovered native path.
    static void IndexGroups(FEntryMap& Groups, const FEntryPtr& Entry)
    {
        InsertGroup(Groups, TAccess::AlternateKey(*Entry), Entry);
        InsertGroup(Groups, TAccess::TemplateId(*Entry), Entry);
    }

    static void DirtyEntryGroups(TSet<FString>& Dirty, const FEntryPtr& Entry)
    {
        Dirty.Add(TAccess::AlternateKey(*Entry));
        Dirty.Add(TAccess::TemplateId(*Entry));
    }
};
