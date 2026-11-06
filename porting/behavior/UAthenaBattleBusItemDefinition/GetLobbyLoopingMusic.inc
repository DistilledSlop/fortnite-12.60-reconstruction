// CONFIDENCE: high; recovered native control flow and typed storage, see native-evidence.json.
// 0x141d4bca0 -> 0x1419b68c0; empty paths return before tracking.
if (LobbyLoopingMusic.IsNull())
    return nullptr;
USoundBase* Result = LobbyLoopingMusic.Get();
if (!Result)
    Result = Cast<USoundBase>(FortLoadDelayedObject(LobbyLoopingMusic.ToSoftObjectPath(), true));
FFortAssets::Get().TrackAsset(Result);
return Result;
