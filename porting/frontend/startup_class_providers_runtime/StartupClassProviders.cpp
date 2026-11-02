#include "../startup_root_added_runtime/RootStartupProviders.h"
#include "UAthenaBroadcastHUDBase.h"
#include "UAthenaHUDBase.h"
#include "UAthenaUIStateWidget_Frontend.h"
#include "UFortUIStateWidget_Login.h"
#include "UFortUIStateWidget_NUI.h"
#include "UStartUpUIStateWidget_SubgameSelect.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Stats/StatsMisc.h"

namespace
{
// 141e12f10: the flag enables optional timing, not the load itself.
UObject* LoadDelayedObject(const FSoftObjectPath& Path, bool bLogLoad)
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

template <typename T>
UClass* LoadTypedClass(const TSoftClassPtr<T>& Class, bool bLogLoad)
{
    if (Class.IsNull())
        return nullptr;

    UClass* Result = Class.Get();
    if (!Result)
        Result = Cast<UClass>(LoadDelayedObject(Class.ToSoftObjectPath(), bLogLoad));
    return Result && Result->IsChildOf(T::StaticClass()) ? Result : nullptr;
}
}

UClass* NativeLoadLoginClass145141970(const TSoftClassPtr<UFortUIStateWidget_Login>& Class, bool bLoad)
{
    return LoadTypedClass(Class, bLoad);
}

UClass* NativeLoadSubgameClass145141b80(const TSoftClassPtr<UStartUpUIStateWidget_SubgameSelect>& Class, bool bLoad)
{
    return LoadTypedClass(Class, bLoad);
}

UClass* NativeLoadFrontendClass145141760(const TSoftClassPtr<UAthenaUIStateWidget_Frontend>& Class, bool bLoad)
{
    return LoadTypedClass(Class, bLoad);
}

UClass* NativeLoadReplayClass145141550(const TSoftClassPtr<UAthenaBroadcastHUDBase>& Class, bool bLoad)
{
    return LoadTypedClass(Class, bLoad);
}

UClass* NativeLoadHUDClass144c7d620(const TSoftClassPtr<UAthenaHUDBase>& Class, bool bLogLoad)
{
    return LoadTypedClass(Class, bLogLoad);
}

UClass* NativeLoadStateClass144da33b0(const TSoftClassPtr<UFortUIStateWidget_NUI>& Class, bool bLogLoad)
{
    return LoadTypedClass(Class, bLogLoad);
}
