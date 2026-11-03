#pragma once

#include "FFortTimeOfDayTheme.h"
#include "UFortTimeOfDayCollection.h"

// 0x141f4fed0. Soft pointers must be copied, including their owned paths.
inline void FortAppendTimeOfDayClasses(
    TArray<TSoftClassPtr<AFortTimeOfDayManager>>& Destination,
    const TArray<TSoftClassPtr<AFortTimeOfDayManager>>& Source)
{
    Destination.Append(Source);
}

// 0x141f5ba50. The cache is rebuilt only when its caller requests it.
inline void FortRebuildValidTimeOfDayManagers(FFortTimeOfDayTheme& Theme)
{
    Theme.ValidTimeOfDayManagers.Empty();
    for (const UFortTimeOfDayCollection* Collection : Theme.TimeOfDayCollections)
    {
        if (Collection)
            FortAppendTimeOfDayClasses(Theme.ValidTimeOfDayManagers, Collection->TimeOfDayManagers);
    }
    FortAppendTimeOfDayClasses(Theme.ValidTimeOfDayManagers, Theme.AdditionalTimeOfDayManagers);

    for (const TSoftClassPtr<AFortTimeOfDayManager>& Prohibited : Theme.ProhibitedTimeOfDayManagers)
    {
        for (int32 Index = 0; Index < Theme.ValidTimeOfDayManagers.Num(); ++Index)
        {
            if (Theme.ValidTimeOfDayManagers[Index].ToSoftObjectPath() == Prohibited.ToSoftObjectPath())
            {
                Theme.ValidTimeOfDayManagers.RemoveAtSwap(Index, 1, true);
                --Index;
            }
        }
    }
}

// 0x141f60280. Return a soft class by value, not a reference to the cache.
inline TSoftClassPtr<AFortTimeOfDayManager> FortSelectTimeOfDayManager(FFortTimeOfDayTheme& Theme)
{
    if (Theme.ValidTimeOfDayManagers.Num() == 0)
        FortRebuildValidTimeOfDayManagers(Theme);
    if (Theme.ValidTimeOfDayManagers.Num() == 0)
        return TSoftClassPtr<AFortTimeOfDayManager>();
    return Theme.ValidTimeOfDayManagers[FMath::RandHelper(Theme.ValidTimeOfDayManagers.Num())];
}
