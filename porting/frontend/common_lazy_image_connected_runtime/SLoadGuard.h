#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Engine/StreamableManager.h"

class SBorder;
struct FSlateBrush;
DECLARE_DELEGATE_OneParam(FNativeLoadGuardStateChanged, bool);
DECLARE_DELEGATE_OneParam(FNativeLoadGuardClassLoaded, UClass*);

// Source aliases for the original private Slate type and its native delegates.
class COMMONUI_API SLoadGuard : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SLoadGuard)
        : _Content(), _LoadingBackgroundBrush(nullptr) {}
        SLATE_DEFAULT_SLOT(FArguments, Content)
        SLATE_ARGUMENT(const FSlateBrush*, LoadingBackgroundBrush)
        SLATE_EVENT(FNativeLoadGuardStateChanged, OnLoadingStateChanged)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    void SetForceShowSpinner(bool bShow);
    bool IsLoading() const { return bIsLoading; }
    void LoadAsset(const TSoftObjectPtr<UObject>& Asset, FNativeLoadGuardClassLoaded OnLoaded);

private:
    void UpdateLoadingState();
    TSoftObjectPtr<UObject> GuardedAsset;
    TSharedPtr<SBorder> ContentBorder;
    TSharedPtr<SBorder> LoadingBorder;
    FNativeLoadGuardStateChanged OnLoadingStateChanged;
    TSharedPtr<FStreamableHandle> GuardedAssetHandle;
    bool bForceShowSpinner = false;
    bool bIsLoading = false;
};
