// CONFIDENCE: high; recovered native control flow and typed storage, see native-evidence.json.
// 0x142546d70 -> 0x14252e040; empty paths return before tracking.
if (PackPersonality.IsNull())
    return nullptr;
UFortPackPersonality* Result = PackPersonality.Get();
if (!Result)
    Result = Cast<UFortPackPersonality>(FortLoadDelayedObject(PackPersonality.ToSoftObjectPath(), true));
FFortAssets::Get().TrackAsset(Result);
return Result;
