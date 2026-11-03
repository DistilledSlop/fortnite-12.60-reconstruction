// Splice into the existing NativeConstruct after Super::NativeConstruct():
// ConfirmationWindow_ConfirmationWidget->NativeWidgetDeactivated.AddUObject(
//     this, &UFortRootViewportLayout::NativeConfirmationDeactivated14514fda0);
// Includes: UFortConfirmationWindow.h, UFortErrorWindow.h,
// UCommonActivatableWidgetQueue.h and ConfirmationImportProviders.h.

void UFortRootViewportLayout::NativeAppendConfirmation145171a60(const FFortDialogDescription_NUI& Description)
{
    ConfirmationQueue.Add(Description);
    if (ConfirmationQueue.Num() == 1)
        NativePresentNextConfirmation145172320();
}

void UFortRootViewportLayout::NativeConfirmationDeactivated14514fda0()
{
    ConfirmationWindow_ConfirmationWidget->SetVisibility(ESlateVisibility::Collapsed);
    if (ConfirmationQueue.Num() > 0)
        ConfirmationQueue.RemoveAt(0, 1, true);
    NativePresentNextConfirmation145172320();
}

void UFortRootViewportLayout::NativePresentNextConfirmation145172320()
{
    if (ConfirmationQueue.Num() > 0)
    {
        ConfirmationWindow_ConfirmationWidget->ConfirmationDescription = ConfirmationQueue[0];
        ConfirmationWindow_ConfirmationWidget->ActivateWidget();
        ConfirmationWindow_ConfirmationWidget->SetVisibility(ESlateVisibility::Visible);
        NativeSetConfirmationGlobalIdle147c577d4(false);
        return;
    }
    NativeSetConfirmationGlobalIdle147c577d4(true);
    const bool bHasModal = NativeGetQueueActiveWidget1407c1910(WidgetQueue_ModalContent) != nullptr
        || ErrorWindow_Errors->IsVisible() || ConfirmationWindow_ConfirmationWidget->IsVisible();
    NativeRootModalStateChanged.Broadcast(bHasModal);
}
