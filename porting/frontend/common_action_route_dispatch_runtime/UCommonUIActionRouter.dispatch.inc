// 14079e590: the normal-dispatch continuation preserves standalone priority.
bool UCommonUIActionRouter::NativeDispatchNormalActionInput(uint8 InputMode, FKey Key, EInputEvent Event)
{
    if (NativeStandaloneActionRoute->ProcessNormalInput(InputMode, Key, Event)) return true;
    return NativeActiveRouteRoot.IsValid() && bNativeActionRoutingEnabled
        && NativeActiveRouteRoot->ProcessNormalInput(InputMode, Key, Event);
}

// 1407e3b40. The original result distinguishes pass-through (1), blocking (2),
// and handled (0). Callers test zero before forwarding to analog-cursor logic.
uint8 UCommonUIActionRouter::NativeProcessActionInput(FKey Key, EInputEvent Event)
{
    const uint8 InputMode = static_cast<uint8>(NativeActionInputConfig.IsSet()
        ? NativeActionInputConfig.GetValue().InputMode : ECommonInputMode::All);
    if (Event == IE_Pressed || Event == IE_Repeat) NativeQueuedActionKeys.AddUnique(Key);
    else if (Event == IE_Released) NativeQueuedActionKeys.Remove(Key);
    auto Result = NativeStandaloneActionRoute->ProcessHoldInput(InputMode, Key, Event);
    if (bNativeActionRoutingEnabled && NativeActiveRouteRoot.IsValid()
        && Result == ECommonActionHoldDispatchResult::Unhandled)
        Result = NativeActiveRouteRoot->ProcessHoldInput(InputMode, Key, Event);
    bool bHandled = Result == ECommonActionHoldDispatchResult::Handled;
    if (!bHandled)
    {
        if (Result == ECommonActionHoldDispatchResult::GeneratePress)
            NativeDispatchNormalActionInput(InputMode, Key, IE_Pressed);
        // Original always dispatches the actual event, even when the synthetic
        // press consumed it; the returned handled result is the actual event's.
        bHandled = NativeDispatchNormalActionInput(InputMode, Key, Event);
    }
    if (bHandled) return 0;
    return NativeShouldBlockActionInput() ? 2 : 1;
}

// 1407ada10. In menu mode viewport cursor capture controls whether an
// unhandled action remains blocked. Other modes return the original true.
bool UCommonUIActionRouter::NativeShouldBlockActionInput() const
{
    if (!NativeActionInputConfig.IsSet()
        || NativeActionInputConfig.GetValue().InputMode != ECommonInputMode::Menu) return true;
    const ULocalPlayer* Player = GetLocalPlayer();
    const auto SlateUser = Player->GetSlateUser();
    if (!SlateUser.IsValid()) return true;
    return Player->ViewportClient
        && SlateUser->DoesWidgetHaveCursorCapture(Player->ViewportClient->GetGameViewportWidget());
}
