UCommonActionWidget::UCommonActionWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    NativeBindingKey = -1;
    NativeHidden = false;
    IconRimBrush = CommonActionWidgetNoResource();
}

TSharedRef<SWidget> UCommonActionWidget::RebuildWidget()
{
    if (!ProgressDynamicMaterial)
    {
        UObject* Resource = ProgressMaterialBrush.GetResourceObject();
        if (!Cast<UMaterialInstanceDynamic>(Resource))
        {
            if (UMaterialInterface* Material = Cast<UMaterialInterface>(Resource))
            {
                ProgressDynamicMaterial = UMaterialInstanceDynamic::Create(Material, nullptr);
                ProgressMaterialBrush.SetResourceObject(ProgressDynamicMaterial);
            }
            else
            {
                ProgressDynamicMaterial = nullptr;
            }
        }
    }

    TSharedRef<SBox> Box = SAssignNew(NativeBox, SBox).HAlign(HAlign_Center).VAlign(VAlign_Center);
    TSharedRef<SImage> Progress = SAssignNew(NativeProgress, SImage).Image(&ProgressMaterialBrush);
    TSharedRef<SImage> Icon = SAssignNew(NativeIcon, SImage).Image(&NativeCachedBrush);
    TSharedRef<SImage> Rim = SAssignNew(NativeRim, SImage).Image(&IconRimBrush);
    Box->SetContent(
        SNew(SOverlay)
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)[Rim]
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)[Icon]
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)[Progress]);
    return Box;
}

void UCommonActionWidget::OnWidgetRebuilt()
{
    Super::OnWidgetRebuilt();
    NativeRefreshInputAction();
    NativeBindInputMethodChanged(true);
}

void UCommonActionWidget::ReleaseSlateResources(bool bReleaseChildren)
{
    NativeProgress.Reset();
    NativeIcon.Reset();
    NativeBox.Reset();
    NativeBindInputMethodChanged(false);
    Super::ReleaseSlateResources(bReleaseChildren);
}
