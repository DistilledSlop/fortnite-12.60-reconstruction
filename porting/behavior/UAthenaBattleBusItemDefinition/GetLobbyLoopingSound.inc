// CONFIDENCE: high; recovered native control flow and typed storage, see native-evidence.json.
// 0x141d4bcb0 -> 0x1419b68c0; empty paths return before tracking.
if (LobbyLoopingSound.IsNull())
    return nullptr;
USoundBase* Result = LobbyLoopingSound.Get();
if (!Result)
    Result = Cast<USoundBase>(FortLoadDelayedObject(LobbyLoopingSound.ToSoftObjectPath(), true));
FFortAssets::Get().TrackAsset(Result);
return Result;
