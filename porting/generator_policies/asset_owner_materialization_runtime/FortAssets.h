#pragma once

#include "CoreMinimal.h"
#include "UObject/GCObject.h"
#include "FortAssetDiagnosticMap.h"
#include "FortAssetLoadDiagnosticState.h"

// One owner for native FFortAssets. This is a source reconstruction, not a
// binary-layout declaration. The diagnostic maps are retained as empty owned
// containers; their mutation/hash policy is not established and no writer is
// exposed until that policy is recovered.
class FORTNITEGAME_API FFortAssets : public FGCObject
{
public:
    static FFortAssets& Get();
    virtual ~FFortAssets();
    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
    virtual FString GetReferencerName() const override;
    virtual bool GetReferencerPropertyName(UObject* Object, FString& OutPropertyName) const override;

    void TrackAsset(UObject* Object);
    static int32 RemovePostStartupAsset(UObject* Object);
    static void AddTemporaryReference(UObject* Object, FName Name);
    static void SetStartupWindowClosedForAlwaysLoadedAssets(bool bClosed);

private:
    friend class UFortAssetManager; // Original diagnostic readers borrow these actual maps.
    FFortAssets();
    FFortAssets(const FFortAssets&) = delete;
    FFortAssets& operator=(const FFortAssets&) = delete;
    void HandlePreLoadMap(const FString& MapName);
    void HandleLoadPackage(const FString& Path, bool bSynchronous);
    void ClearTemporaryAssets();

    // Types and empty construction/destruction are supported; equality, hash,
    // and mutation behavior remain unverified. Do not add map operations yet.
    FFortAssetDiagnosticMap DiagnosticAt10;
    FFortAssetDiagnosticMap DiagnosticAt60;
    TSet<UObject*> AlwaysLoadedAssets;
    TSet<UObject*> PostStartupAssets;
    TMap<UObject*, FName> TemporaryReferences;
    mutable FCriticalSection CriticalSection;
    bool bStartupWindowClosedForAlwaysLoadedAssets = false;
    FFortAssetLoadDiagnosticState DiagnosticState;
};
