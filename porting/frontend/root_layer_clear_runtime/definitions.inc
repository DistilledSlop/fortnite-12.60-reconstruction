void UFortRootViewportLayout::NativeClearLayers498()
{
    if (UFortMcpContext* Context = NativeGetMcpContext1425dc880(this))
        NativeCancelMcpPendingRequest1421cb230(Context);

    ProgressModal_ControllerDisconnected->SetVisibility(ESlateVisibility::Collapsed);
    ProgressModal_ControllerDisconnected->DeactivateWidget();
    NativeClearModalQueue1407ae7f0(WidgetQueue_ModalContent);
    ConfirmationQueue.Empty();
    NativeClearConfirmation144f664f0(ConfirmationWindow_ConfirmationWidget);
    NativeClearErrors144d74df0(ErrorWindow_Errors);
}
