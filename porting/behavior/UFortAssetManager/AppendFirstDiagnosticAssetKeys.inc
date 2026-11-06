// CONFIDENCE: HIGH
// Original 141dfb390, using the same live singleton maps.
const int32 EntryCount = FFortAssets::Get().DiagnosticAt10.Num();
if (EntryCount > Out.Max())
{
    Out.Reserve(EntryCount);
}
for (const auto& Entry : FFortAssets::Get().DiagnosticAt10)
{
    Out.Add(Entry.Key);
}
