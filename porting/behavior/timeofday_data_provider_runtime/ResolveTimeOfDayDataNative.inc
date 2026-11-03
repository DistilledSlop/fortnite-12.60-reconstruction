// CONFIDENCE: HIGH
// 0x1424e4940 -> 0x1424ce5d0. An empty path returns before tracking.
if (FortTimeOfDayData.IsNull())
    return nullptr;
UFortTimeOfDayData* Data = FortTimeOfDayData.Get();
if (!Data)
    Data = Cast<UFortTimeOfDayData>(FortLoadDelayedObject(FortTimeOfDayData.ToSoftObjectPath(), true));
FFortAssets::Get().TrackAsset(Data);
return Data;
