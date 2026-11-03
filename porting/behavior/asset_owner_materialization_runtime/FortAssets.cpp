#include "FortAssets.h"
#include "UFortAssetManager.h"
#include "SharedFortGameLog.h"
#include "Misc/CoreDelegates.h"
#include "Misc/ScopeLock.h"
#include "UObject/UObjectGlobals.h"

FFortAssets& FFortAssets::Get()
{
    static FFortAssets Instance;
    return Instance;
}

FFortAssets::FFortAssets()
{
    FCoreUObjectDelegates::PreLoadMap.AddRaw(this, &FFortAssets::HandlePreLoadMap);
    FCoreDelegates::OnSyncLoadPackage.AddRaw(this, &FFortAssets::HandleLoadPackage, true);
}

FFortAssets::~FFortAssets()
{
    FCoreUObjectDelegates::PreLoadMap.RemoveAll(this);
    FCoreDelegates::OnAsyncLoadingFlush.RemoveAll(this);
    FCoreDelegates::OnAsyncLoadPackage.RemoveAll(this);
    FCoreDelegates::OnSyncLoadPackage.RemoveAll(this);
}

void FFortAssets::TrackAsset(UObject* Object)
{
    FScopeLock Lock(&CriticalSection);
    if (!bStartupWindowClosedForAlwaysLoadedAssets)
    {
        AlwaysLoadedAssets.Add(Object);
    }
    else if (!AlwaysLoadedAssets.Contains(Object))
    {
        PostStartupAssets.Add(Object);
    }
}

int32 FFortAssets::RemovePostStartupAsset(UObject* Object)
{
    return Get().PostStartupAssets.Remove(Object);
}

void FFortAssets::AddTemporaryReference(UObject* Object, FName Name)
{
    if (Object)
    {
        Get().TemporaryReferences.Add(Object, Name);
    }
}

void FFortAssets::AddReferencedObjects(FReferenceCollector& Collector)
{
    {
        FScopeLock Lock(&CriticalSection);
        Collector.AddReferencedObjects(AlwaysLoadedAssets);
        Collector.AddReferencedObjects(PostStartupAssets);
    }
    Collector.AddReferencedObjects(TemporaryReferences);
}

void FFortAssets::ClearTemporaryAssets()
{
    FScopeLock Lock(&CriticalSection);
    PostStartupAssets.Empty(0);
    TemporaryReferences.Empty(0);
}

void FFortAssets::HandlePreLoadMap(const FString& /*MapName*/)
{
    ClearTemporaryAssets();
}

void FFortAssets::HandleLoadPackage(const FString& Path, bool bSynchronous)
{
    UFortAssetManager::NativeHandleLoadDiagnostic(DiagnosticState, Path, bSynchronous);
}

void FFortAssets::SetStartupWindowClosedForAlwaysLoadedAssets(bool bClosed)
{
    FFortAssets& Assets = Get();
    FScopeLock Lock(&Assets.CriticalSection);
    if (Assets.bStartupWindowClosedForAlwaysLoadedAssets == bClosed)
    {
        UE_LOG(LogFort, Error, TEXT("SetStartupWindowClosedForAlwaysLoadedAssets called with already set value!"));
    }
    else
    {
        Assets.bStartupWindowClosedForAlwaysLoadedAssets = bClosed;
    }
}

FString FFortAssets::GetReferencerName() const
{
    return TEXT("FFortAssets");
}

bool FFortAssets::GetReferencerPropertyName(UObject* Object, FString& OutPropertyName) const
{
    if (!Object)
    {
        return false;
    }

    {
        FScopeLock Lock(&CriticalSection);
        if (AlwaysLoadedAssets.Contains(Object))
        {
            OutPropertyName = TEXT("AlwaysLoadedAssets");
            return true;
        }
        if (PostStartupAssets.Contains(Object))
        {
            OutPropertyName = TEXT("LoadedAssets");
            return true;
        }
    }

    const FName* FoundName = TemporaryReferences.Find(Object);
    if (!FoundName)
    {
        return false;
    }
    const FName Name = *FoundName;
    if (Name == NAME_None)
    {
        return false;
    }
    OutPropertyName = TEXT("GCPreventionPool.") + Name.ToString();
    return true;
}
