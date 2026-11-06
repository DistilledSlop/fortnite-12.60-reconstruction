// CONFIDENCE: high; recovered native control flow and typed storage, see native-evidence.json.
// 0x141d4a960 -> 0x1419b68c0; empty paths return before tracking.
if (IgnitionSound.IsNull())
    return nullptr;
USoundBase* Result = IgnitionSound.Get();
if (!Result)
    Result = Cast<USoundBase>(FortLoadDelayedObject(IgnitionSound.ToSoftObjectPath(), true));
FFortAssets::Get().TrackAsset(Result);
return Result;
