// Required includes in router.cpp: CommonActionBindingRoute.h,
// UCommonInputSubsystem.h, Engine/{LocalPlayer,GameViewportClient}.h,
// GameFramework/PlayerController.h, Framework/Application/{SlateApplication,SlateUser}.h,
// Slate/SGameLayerManager.h, Widgets/SViewport.h.

void UCommonUIActionRouter::NativeSetActiveActionRouteRoot(const TSharedPtr<FCommonActionRouteRoot>& Root)
{
    if (NativeActiveRouteRoot.IsValid())
    {
        NativeActiveRouteRoot->OnSelectedNodeChanged.Unbind();
        NativeActiveRouteRoot->SetRoutingActive(false);
    }
    if (!bNativeActionRoutingBlocked && bNativeActionRoutingEnabled)
    {
        NativeActiveRouteRoot = Root;
        if (Root.IsValid())
        {
            Root->SetRoutingActive(true);
            Root->OnSelectedNodeChanged.BindUObject(this, &UCommonUIActionRouter::NativeHandleActionRouteSelectionChanged);
        }
    }
    else NativeActiveRouteRoot.Reset();
}

void UCommonUIActionRouter::NativeHandleActionRouteSelectionChanged()
{
    NativeActionBindingsChanged.Broadcast();
}

void UCommonUIActionRouter::NativeApplyActionRouteInputConfig(const FCommonActionRouteInputConfig& Config)
{
    if (NativeActionInputConfig.IsSet() && NativeActionInputConfig.GetValue() == Config) return;
    const bool bHadConfig = NativeActionInputConfig.IsSet();
    const ECommonInputMode OldMode = bHadConfig ? NativeActionInputConfig.GetValue().InputMode : ECommonInputMode::All;
    NativeActionInputConfig = Config; // state precedes every external effect
    auto* Player = GetLocalPlayer();
    auto* ViewportClient = Player->ViewportClient;
    if (ViewportClient)
    {
        const auto Viewport = ViewportClient->GetGameViewportWidget();
        if (Viewport.IsValid())
        {
            if (auto* Controller = Player->GetPlayerController(GetWorld()))
            {
                const auto OldCapture = ViewportClient->CaptureMouseOnClick();
                const bool bWasPermanent = OldCapture == EMouseCaptureMode::CapturePermanently
                    || OldCapture == EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown;
                ViewportClient->SetCaptureMouseOnClick(Config.MouseCaptureMode);
                ViewportClient->SetHideCursorDuringCapture(Config.bHideCursorDuringCapture
                    && !RouteShouldShowCursorForTouch(ViewportClient));
                auto& Operations = Player->GetSlateOperations();
                const uint8 Capture = static_cast<uint8>(Config.MouseCaptureMode);
                if (Capture == 1 || Capture == 2)
                {
                    ViewportClient->SetMouseLockMode(EMouseLockMode::LockOnCapture);
                    Controller->bShowMouseCursor = RouteShouldShowCursorForTouch(ViewportClient);
                    Operations.UseHighPrecisionMouseMovement(Viewport.ToSharedRef());
                    Operations.SetUserFocus(Viewport.ToSharedRef(), EFocusCause::SetDirectly, false);
                    Operations.LockMouseToWidget(Viewport.ToSharedRef());
                }
                else if (Capture <= 4)
                {
                    ViewportClient->SetMouseLockMode(EMouseLockMode::DoNotLock);
                    Controller->bShowMouseCursor = true;
                    Operations.ReleaseMouseLock();
                    Operations.ReleaseMouseCapture();
                    if (bHadConfig && bWasPermanent
                        && Player->GetSubsystem<UCommonInputSubsystem>()->GetCurrentInputType() != ECommonInputType::Touch)
                    {
                        const auto SlateUser = Player->GetSlateUser();
                        const auto Layers = ViewportClient->GetGameLayerManager();
                        if (SlateUser.IsValid() && Layers.IsValid())
                        {
                            const auto Geometry = Layers->GetPlayerWidgetHostGeometry(Player);
                            SlateUser->SetCursorPosition(Geometry.LocalToAbsolute(Geometry.GetLocalSize() * 0.5f));
                        }
                    }
                }
            }
        }
    }
    if (OldMode != Config.InputMode) NativeActionInputModeChanged.Broadcast(Config.InputMode);
}
