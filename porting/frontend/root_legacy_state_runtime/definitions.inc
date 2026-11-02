// 14516f380. Selector bytes are preserved until the native enum is joined.
void UFortRootViewportLayout::NativeSetLegacyState14516f380(uint8 Selector)
{
    UClass* StateClass = UFortUIDataConfiguration::NativeGet144db86e0()
        ->NativeGetLegacyStateClass260(Selector);
    NativeChooseLegacyState480(Selector, StateClass);
    UFortUIStateWidget_NUI* PreviousWidget = CurrentLegacyStateWidget;
    NativePendingLegacyState2a9 = Selector;
    if (PreviousWidget)
    {
        PreviousWidget->NativeDeactivateLegacyState518();
        NativeClearLayers498();
    }

    UFortUIStateWidget_NUI* NewWidget = nullptr;
    if (StateClass && StateClass->IsChildOf(UFortUIStateWidget_NUI::StaticClass()))
        NewWidget = CreateWidget<UFortUIStateWidget_NUI>(GetOwningPlayer(), StateClass, NAME_None);

    const uint8 PreviousState = NativeLegacyState2a8;
    NativeLegacyState2a8 = NativePendingLegacyState2a9;
    CurrentLegacyStateWidget = NewWidget;
    NativePendingLegacyState2a9 = 0;
    UE_LOG(LogFortUI, Display,
        TEXT("[UFortRootViewportLayout::SetLegacyUIState] Switching states from: %s to %s"),
        *NativeLegacyStateName14514e200(PreviousState),
        *NativeLegacyStateName14514e200(NativeLegacyState2a8));
    NativeAttachState145174c00(CurrentLegacyStateWidget);
    // Re-read the member after attachment, as the original does.
    if (CurrentLegacyStateWidget)
    {
        CurrentLegacyStateWidget->NativeActivateLegacyState510(PreviousState);
        if (NativeLegacyState2a8 == 9 && NotificationManager)
        {
            if (UFortUINotificationQueue* Queue = NotificationManager->NativeGetQueue144e00700(0))
                ToastDisplayArea_ToastHandler->NativeSetQueue144fb3e40(Queue);
        }
    }
    NativeRemoveWidget145174fa0(PreviousWidget);
}

// 145174c00 accepts either legacy or new state widgets through their common base.
void UFortRootViewportLayout::NativeAttachState145174c00(UUserWidget* StateWidget)
{
    if (NativeRootBox2e8.IsValid())
        NativeRootBox2e8->SetContent(StateWidget ? StateWidget->TakeWidget() : SNullWidget::NullWidget);
}

// 14514e200. FindObject result is cached even when lookup fails.
FString UFortRootViewportLayout::NativeLegacyStateName14514e200(uint8 State) const
{
    static UEnum* StateEnum = FindObject<UEnum>(ANY_PACKAGE, TEXT("EFortUIState"), true);
    return StateEnum ? StateEnum->GetNameStringByValue(State) : FString(TEXT("Unknown"));
}
