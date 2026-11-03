// CONFIDENCE: HIGH
// Assigned static UFortAssetManager helper for 0x141dfb390.
const int32 EntryCount = MapAt10.Num();
if (EntryCount > Out.Max())
{
    Out.Reserve(EntryCount);
}
for (const auto& Entry : MapAt10)
{
    Out.Add(Entry.Key);
}
