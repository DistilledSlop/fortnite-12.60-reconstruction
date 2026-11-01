UCommonActivatableWidget::UCommonActivatableWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Original ctor 140808180 and original Bool-setter-to-field bindings.
    bAutoActivate = false;
    bIsBackHandler = false;
    bSupportsActivationFocus = true;
    bIsModal = false;
    bAutoRestoreFocus = false;
    bSetVisibilityOnActivated = false;
    ActivatedVisibility = ESlateVisibility::SelfHitTestInvisible;
    bSetVisibilityOnDeactivated = false;
    DeactivatedVisibility = ESlateVisibility::Collapsed;
    bIsActive = false;
}
void UCommonActivatableWidget::ActivateWidget()
{
    CommonActivationCore::Activate([this] { return bIsActive; }, [this] { NativeActivateRequest(); });
}
void UCommonActivatableWidget::DeactivateWidget()
{
    CommonActivationCore::Deactivate([this] { return bIsActive; }, [this] { NativeDeactivateRequest(); });
}
bool UCommonActivatableWidget::IsActivated() const { return bIsActive; }
void UCommonActivatableWidget::NativeActivateRequest()
{
    CommonActivationCore::SetActive(true, [this](bool Active) { bIsActive = Active; }, [this] { NativeHandleActivated(); });
}
void UCommonActivatableWidget::NativeDeactivateRequest()
{
    CommonActivationCore::SetActive(false, [this](bool Active) { bIsActive = Active; }, [this] { NativeHandleDeactivated(); });
}
void UCommonActivatableWidget::NativeHandleActivated()
{
    CommonActivationCore::Notify(true, [this] { return bIsActive; },
        [this] { if (bSetVisibilityOnActivated) SetVisibility(ActivatedVisibility); },
        [this] { BP_OnActivated(); },
        [this] { NativeWidgetActivated.Broadcast(); },
        [this] { BP_OnWidgetActivated.Broadcast(); });
}
void UCommonActivatableWidget::NativeHandleDeactivated()
{
    CommonActivationCore::Notify(false, [this] { return bIsActive; },
        [this] { if (bSetVisibilityOnDeactivated) SetVisibility(DeactivatedVisibility); },
        [this] { BP_OnDeactivated(); },
        [this] { NativeWidgetDeactivated.Broadcast(); },
        [this] { BP_OnWidgetDeactivated.Broadcast(); });
}
