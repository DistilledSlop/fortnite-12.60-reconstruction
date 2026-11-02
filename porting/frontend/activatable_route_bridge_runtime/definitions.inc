TOptional<TOptional<uint8>> UCommonActivatableWidget::NativeGetActionRouteOverride() const
{
    return TOptional<TOptional<uint8>>();
}

TOptional<FCommonActionRouteInputConfig> UCommonActivatableWidget::NativeGetActionRouteInputConfig() const
{
    return TOptional<FCommonActionRouteInputConfig>();
}

UWidget* UCommonActivatableWidget::NativeGetActionRouteDesiredFocusTarget() const
{
    return BP_GetDesiredFocusTarget();
}

void UCommonActivatableWidget::ReleaseSlateResources(bool bReleaseChildren)
{
    Super::ReleaseSlateResources(bReleaseChildren);
    NativeWidgetReleased.Broadcast();
}

TSharedRef<SWidget> UCommonActivatableWidget::RebuildWidget()
{
    GetCommonActivatableWidgetRebuilt().Broadcast(this);
    return Super::RebuildWidget();
}

void UCommonActivatableWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (bIsBackHandler)
    {
        FDataTableRowHandle BackAction;
        UCommonInputSettings* Settings = GetCommonInputSettings();
        UClass* InputDataClass = Settings->InputDataClass.Get();
        if (InputDataClass && InputDataClass->IsChildOf(UCommonUIInputData::StaticClass()))
        {
            UObject* Default = InputDataClass->GetDefaultObject(false);
            if (!Default)
            {
                InputDataClass->GetDefaultObject();
                Default = InputDataClass->GetDefaultObject(false);
            }
            if (Default) BackAction = static_cast<UCommonUIInputData*>(Default)->DefaultBackAction;
        }
        FCommonActionBindingDescriptor Descriptor{};
        Descriptor.Row = BackAction;
        Descriptor.InputMode = 0;
        Descriptor.Event = 0;
        Descriptor.Flag20 = 0;
        Descriptor.Flag21 = 1;
        Descriptor.Flag22 = 1;
        Descriptor.Click = FSimpleDelegate::CreateUObject(this, &UCommonActivatableWidget::NativeDispatchBackAction);
        NativeRegisterActionBinding(Descriptor);
    }
    if (bAutoActivate && !bIsActive) NativeActivateRequest();
}

void UCommonActivatableWidget::NativeDispatchBackAction()
{
    NativeHandleBackAction();
}

bool UCommonActivatableWidget::NativeHandleBackAction()
{
    if (!bIsBackHandler) return false;
    if (!BP_OnHandleBackAction() && bIsActive) NativeDeactivateRequest();
    return true;
}
