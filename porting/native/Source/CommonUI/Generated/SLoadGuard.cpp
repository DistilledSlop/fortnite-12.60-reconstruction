#include "SLoadGuard.h"
#include "CommonUIModuleRuntime.h"
#include "UCommonUISettings.h"
#include "Engine/AssetManager.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"

void SLoadGuard::Construct(const FArguments& InArgs)
{
    // The CommonLazyImage construction supplies no loading text or text style.
    // Its private load guard's default text is empty and alignment is Center.
    OnLoadingStateChanged = InArgs._OnLoadingStateChanged;
    ChildSlot
    [
        SNew(SOverlay)
        + SOverlay::Slot()
        [
            SAssignNew(ContentBorder, SBorder)
            .Padding(0).BorderImage(nullptr)
            .Visibility(EVisibility::SelfHitTestInvisible)
            [ InArgs._Content.Widget ]
        ]
        + SOverlay::Slot()
        [
            SAssignNew(LoadingBorder, SBorder)
            .HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(0)
            .BorderImage(InArgs._LoadingBackgroundBrush)
            .Visibility(EVisibility::Collapsed)
            [
                SNew(SScaleBox).Stretch(EStretch::ScaleToFit)
                .StretchDirection(EStretchDirection::DownOnly)
                .HAlign(HAlign_Fill).VAlign(VAlign_Fill)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left).VAlign(VAlign_Center)
                    [ SNew(SImage).Image(&NativeGetCommonUISettings()->DefaultThrobberBrush) ]
                    + SHorizontalBox::Slot().FillWidth(1).HAlign(HAlign_Left).VAlign(VAlign_Center)
                    [ SNew(STextBlock).Text(FText::GetEmpty()) ]
                ]
            ]
        ]
    ];
    bForceShowSpinner = false;
    UpdateLoadingState();
}

void SLoadGuard::SetForceShowSpinner(bool bShow)
{
    bForceShowSpinner = bShow;
    UpdateLoadingState();
}

void SLoadGuard::UpdateLoadingState()
{
    const bool bLoading = GuardedAssetHandle.IsValid() || bForceShowSpinner;
    if (bLoading != bIsLoading)
    {
        bIsLoading = bLoading;
        OnLoadingStateChanged.ExecuteIfBound(bIsLoading);
        ContentBorder->SetVisibility(bIsLoading ? EVisibility::Collapsed : EVisibility::SelfHitTestInvisible);
        LoadingBorder->SetVisibility(bIsLoading ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed);
    }
}

void SLoadGuard::LoadAsset(const TSoftObjectPtr<UObject>& Asset, FNativeLoadGuardClassLoaded OnLoaded)
{
    const bool bNewPath = GuardedAsset.ToSoftObjectPath() != Asset.ToSoftObjectPath();
    if (bNewPath && GuardedAssetHandle.IsValid())
    {
        GuardedAssetHandle->CancelHandle();
        GuardedAssetHandle.Reset();
    }
    GuardedAsset = Asset;
    if (UClass* LoadedClass = Cast<UClass>(GuardedAsset.Get()))
    {
        OnLoaded.ExecuteIfBound(LoadedClass);
    }
    else if (bNewPath && !GuardedAsset.IsNull())
    {
        TWeakPtr<SLoadGuard> WeakThis = SharedThis(this);
        GuardedAssetHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
            GuardedAsset.ToSoftObjectPath(), FStreamableDelegate::CreateLambda([WeakThis, OnLoaded]()
            {
                const TSharedPtr<SLoadGuard> Guard = WeakThis.Pin();
                if (Guard.IsValid() && Cast<UClass>(Guard->GuardedAsset.Get()))
                {
                    Guard->GuardedAssetHandle.Reset();
                    Guard->UpdateLoadingState();
                    OnLoaded.ExecuteIfBound(Cast<UClass>(Guard->GuardedAsset.Get()));
                }
            }), 100, false, false, TEXT("SLoadGuard::LoadAsset"));
    }
    UpdateLoadingState();
}
