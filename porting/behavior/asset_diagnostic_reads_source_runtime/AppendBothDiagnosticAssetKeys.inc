// CONFIDENCE: HIGH
// Assigned static UFortAssetManager helper for 0x141df6860.
const int32 FirstCount = FFortAssets::Get().DiagnosticAt10.Num();
const int32 EntryCount = FirstCount + FFortAssets::Get().DiagnosticAt60.Num();
if (EntryCount > Out.Max())
{
    Out.Reserve(EntryCount);
}
FFortAssets& Assets = FFortAssets::Get();
for (const auto& Entry : Assets.DiagnosticAt10)
{
    Out.Add(Entry.Key);
}
for (const auto& Entry : Assets.DiagnosticAt60)
{
    Out.Add(Entry.Key);
}
