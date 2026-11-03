// CONFIDENCE: high; see native-evidence.json for captured native control flow.
// Native 0x14208efa0; reflected wrapper 0x1429bfce0.
if (!PlayerState) return INDEX_NONE;
FUniqueNetIdRepl PlayerId = PlayerState->GetUniqueId();
return GetBucketForPlayerId(PlayerId);
