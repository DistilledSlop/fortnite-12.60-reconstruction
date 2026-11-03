#include "McpProfileBackend.h"

namespace
{
// Original comparator folds ASCII and compares the remaining UTF-16 units exactly.
int32 CompareItemIds(const FString& Left, const FString& Right)
{
    const TCHAR* A = *Left;
    const TCHAR* B = *Right;
    for (;; ++A, ++B)
    {
        uint32 X = static_cast<uint32>(*A);
        uint32 Y = static_cast<uint32>(*B);
        if (X == Y)
        {
            if (X == 0) return 0;
            continue;
        }
        if ((X | Y) < 0x80)
        {
            if (X >= 'A' && X <= 'Z') X += 'a' - 'A';
            if (Y >= 'A' && Y <= 'Z') Y += 'a' - 'A';
        }
        if (X != Y) return X < Y ? -1 : 1;
    }
}
}

FMcpProfileBackend::FMcpProfileBackend() = default;
FMcpProfileBackend::~FMcpProfileBackend() = default;

const FMcpProfileBackend::FItemArray* FMcpProfileBackend::FindGroup(const FString& Key) const
{
    return FindBaseGroup(Key);
}

const FMcpProfileBackend::FItemArray* FMcpProfileBackend::FindBaseGroup(const FString& Key) const
{
    return Groups.Find(Key);
}

void FMcpProfileBackend::InsertItem(const FItemPtr& Item)
{
    // As in the recovered path, Item must be valid.
    ItemsById.Add(Item->ItemId, Item);
    InsertGroup(Groups, Item->AlternateDefinitionKey, Item);
    InsertGroup(Groups, Item->TemplateId, Item);
}

void FMcpProfileBackend::InsertGroup(FGroupMap& InGroups, const FString& Key, const FItemPtr& Item)
{
    FItemArray& Group = InGroups.FindOrAdd(Key);
    int32 First = 0;
    int32 Count = Group.Num();
    while (Count > 0)
    {
        const int32 Half = Count / 2;
        const int32 Mid = First + Half;
        if (CompareItemIds(Group[Mid]->ItemId, Item->ItemId) < 0)
        {
            First = Mid + 1;
            Count -= Half + 1;
        }
        else
        {
            Count = Half;
        }
    }
    if (First == Group.Num() || CompareItemIds(Group[First]->ItemId, Item->ItemId) != 0)
    {
        Group.Insert(Item, First);
    }
}
