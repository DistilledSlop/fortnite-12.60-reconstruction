// CONFIDENCE: high; see native-evidence.json for captured native control flow.
// Native 0x14208eee0; reflected wrapper 0x1429bfbe0.
if (!PlayerId.IsValid()) return INDEX_NONE;
for (int32 Index = 0; Index < PlayerBuckets.Num(); ++Index)
{
    if (PlayerBuckets[Index].PlayerIds.Contains(PlayerId)) return Index;
}
return INDEX_NONE;
