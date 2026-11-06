// CONFIDENCE: high; recovered native control flow and typed storage, see native-evidence.json.
// 0x141d49b60 -> 0x1419b68c0; empty paths return before tracking.
if (FlightLoopingMusic.IsNull())
    return nullptr;
USoundBase* Result = FlightLoopingMusic.Get();
if (!Result)
    Result = Cast<USoundBase>(FortLoadDelayedObject(FlightLoopingMusic.ToSoftObjectPath(), true));
FFortAssets::Get().TrackAsset(Result);
return Result;
