void UCommonActivatableWidgetQueue::NativeOnWidgetAdded1407ecbe0(UCommonActivatableWidget* Widget)
{
    if (!NativeAnimatedSwitcher1c8.IsValid())
        return;
    NativeAnimatedSwitcher1c8->AddSlot(1)[Widget->TakeWidget()];
    const int32 NumWidgets = NativeAnimatedSwitcher1c8->GetNumWidgets();
    if (NumWidgets == 2)
        NativeSetActiveIndex1407f9d70(NumWidgets - 1, false);
    else if (DisplayedWidget && !DisplayedWidget->bIsActive && NumWidgets == 3)
        NativeSetActiveIndex1407f9d70(NumWidgets - 2, true);
}
