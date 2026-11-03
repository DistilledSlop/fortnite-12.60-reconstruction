// CONFIDENCE: HIGH
// Assigned static UFortAssetManager helper for 0x141df6860.
const int32 EntryCount = MapAt10.Num() + MapAt60.Num();
if (EntryCount > Out.Max())
{
    Out.Reserve(EntryCount);
}
for (const auto& Entry : MapAt10)
{
    Out.Add(Entry.Key);
}
for (const auto& Entry : MapAt60)
{
    Out.Add(Entry.Key);
}
