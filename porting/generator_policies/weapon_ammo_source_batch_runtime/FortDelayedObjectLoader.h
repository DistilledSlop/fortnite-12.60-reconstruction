#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Stats/StatsMisc.h"
#include "UObject/SoftObjectPath.h"

// Native 0x141e12f10. The flag controls timing only, never whether to load.
inline UObject* FortLoadDelayedObject(const FSoftObjectPath& Path, bool bLogLoad)
{
    if (Path.IsNull())
        return nullptr;
    static const bool bShowDelayLoadTimes = FParse::Param(FCommandLine::Get(), TEXT("ShowDelayLoadTimes"));
    TUniquePtr<FScopeLogTime> Timer;
    if (bLogLoad && bShowDelayLoadTimes)
    {
        const FString Label = FString::Printf(TEXT("Loaded delay-load asset %s"), *Path.ToString());
        Timer = MakeUnique<FScopeLogTime>(*Label, nullptr, FConditionalScopeLogTime::ScopeLog_Seconds);
    }
    return UAssetManager::IsValid()
        ? UAssetManager::GetStreamableManager().LoadSynchronous(Path, false, nullptr)
        : Path.TryLoad();
}
