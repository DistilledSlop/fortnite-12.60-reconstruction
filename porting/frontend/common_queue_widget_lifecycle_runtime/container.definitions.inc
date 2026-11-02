UCommonActivatableWidget* UCommonActivatableWidgetContainerBase::NativeAddWidget1407aa4c0(
    UClass* WidgetClass, TFunctionRef<void(UCommonActivatableWidget*)> Initializer)
{
    UCommonActivatableWidget* Widget = GeneratedWidgetsPool.GetOrCreateInstance<UCommonActivatableWidget>(WidgetClass);
    if (Widget)
    {
        Initializer(Widget);
        if (!WidgetList.Contains(Widget))
        {
            WidgetList.Add(Widget);
            NativeOnWidgetAdded1407ecbe0(Widget);
        }
    }
    return Widget;
}

void UCommonActivatableWidgetContainerBase::NativeOnWidgetAdded1407ecbe0(UCommonActivatableWidget* Widget)
{
    // Original base virtual slot +0x2c0 is the empty function 0x1406801c0.
}

void UCommonActivatableWidgetContainerBase::NativeRemoveWidget1407f1000(UCommonActivatableWidget* Widget)
{
    UCommonActivatableWidget* Active = nullptr;
    if (NativeAnimatedSwitcher1c8.IsValid())
    {
        TSharedPtr<SWidget> ActiveSlate = NativeAnimatedSwitcher1c8->GetActiveWidget();
        if (ActiveSlate.IsValid() && ActiveSlate != SNullWidget::NullWidget && ActiveSlate->GetType() == FName(TEXT("SObjectWidget")))
            Active = Cast<UCommonActivatableWidget>(StaticCastSharedPtr<SObjectWidget>(ActiveSlate)->GetWidgetObject());
    }
    if (Widget == Active)
    {
        if (Widget->bIsActive)
            Widget->DeactivateWidget();
        else
            NativePendingDeactivation1e8 = true;
    }
    else
    {
        TSharedPtr<SWidget> SlateWidget = Widget->GetCachedWidget();
        if (SlateWidget.IsValid() && NativeAnimatedSwitcher1c8.IsValid())
            NativeRemoveSlateWidget1407eefa0(SlateWidget.ToSharedRef());
    }
}

UCommonActivatableWidget* NativeAddQueueWidget1407aa4c0(UCommonActivatableWidgetQueue* Queue,
    UClass* WidgetClass, TFunctionRef<void(UCommonActivatableWidget*)> Initializer)
{
    return Queue->NativeAddWidget1407aa4c0(WidgetClass, Initializer);
}

void NativeRemoveQueueWidget1407f1000(UCommonActivatableWidgetQueue* Queue, UCommonActivatableWidget* Widget)
{
    Queue->NativeRemoveWidget1407f1000(Widget);
}
