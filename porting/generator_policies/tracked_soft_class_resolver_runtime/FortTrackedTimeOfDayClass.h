#pragma once

#include "FortAssets.h"
#include "FortDelayedObjectLoader.h"
#include "AFortTimeOfDayManager.h"

// 0x14192c570. TSubclassOf retains the raw loaded UClass in its storage;
// validation occurs on Get(), including for the tracking argument below.
inline TSubclassOf<AFortTimeOfDayManager> FortResolveTrackedTimeOfDayClass(
    const TSoftClassPtr<AFortTimeOfDayManager>& SoftClass, bool bLogLoadTime)
{
    UClass* RawClass = nullptr;
    if (!SoftClass.IsNull())
    {
        RawClass = SoftClass.Get();
        if (!RawClass)
            RawClass = Cast<UClass>(FortLoadDelayedObject(SoftClass.ToSoftObjectPath(), bLogLoadTime));
    }
    TSubclassOf<AFortTimeOfDayManager> Result(RawClass);
    FFortAssets::Get().TrackAsset(Result.Get());
    return Result;
}
