// CONFIDENCE: high; recovered native control flow and typed storage, see native-evidence.json.
// 0x141d49b70 -> 0x1419b68c0; empty paths return before tracking.
if (FlightLoopingSound.IsNull())
    return nullptr;
USoundBase* Result = FlightLoopingSound.Get();
if (!Result)
    Result = Cast<USoundBase>(FortLoadDelayedObject(FlightLoopingSound.ToSoftObjectPath(), true));
FFortAssets::Get().TrackAsset(Result);
return Result;
