// Read-side proposal only. Map ownership, mutation, and exact key policy remain
// unresolved; the names below identify the two owner members by offset.
// The original takes no lock and appends, including cross-map duplicates.
void AppendAssetsAt10(TArray<FSoftObjectPath>& Out) const
{
    Out.Reserve(MapAt10.Num());
    for (const auto& Entry : MapAt10)
    {
        Out.Add(Entry.Key);
    }
}

void AppendAssetsAt10And60(TArray<FSoftObjectPath>& Out) const
{
    Out.Reserve(MapAt10.Num() + MapAt60.Num());
    for (const auto& Entry : MapAt10)
    {
        Out.Add(Entry.Key);
    }
    for (const auto& Entry : MapAt60)
    {
        Out.Add(Entry.Key);
    }
}
