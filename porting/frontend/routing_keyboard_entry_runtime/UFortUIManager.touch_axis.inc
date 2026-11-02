// Includes: FFortRootViewportLayoutInfo.h, UFortLocalPlayer.h,
// UCommonUIActionRouter.h, Engine/LocalPlayer.h, InputCoreTypes.h.
// 145156aa0 and 145156360. The original callbacks leave Reply unchanged on
// every early return; the viewport initializes it to Unhandled before calling.
ULocalPlayer* UFortUIManager::NativeFindRoutedLocalPlayer(int32 ControllerId) const
{
    for (const FFortRootViewportLayoutInfo& Entry : RootViewportLayouts)
        if (Entry.LocalPlayer && Entry.LocalPlayer->GetControllerId() == ControllerId)
            return Entry.LocalPlayer;
    return nullptr;
}

void UFortUIManager::NativeRouteTouch(int32 ControllerId, uint32 Handle, ETouchType::Type Type,
    const FVector2D& TouchLocation, FReply& Reply)
{
    if (Handle >= EKeys::NUM_TOUCH_KEYS || !EKeys::TouchKeys[Handle].IsValid()) return;
    ULocalPlayer* Player = NativeFindRoutedLocalPlayer(ControllerId);
    UCommonUIActionRouter* Router = Player ? Player->GetSubsystem<UCommonUIActionRouter>() : nullptr;
    if (!Router) return;
    const EInputEvent Event = Type == ETouchType::Began ? IE_Pressed
        : Type == ETouchType::Ended ? IE_Released : IE_Repeat;
    if (Router->NativeProcessActionInput(EKeys::TouchKeys[Handle], Event) != 2)
        Reply = FReply::Handled();
}

void UFortUIManager::NativeRouteAxis(int32 ControllerId, FKey Key, float Delta, FReply& Reply)
{
    ULocalPlayer* Player = NativeFindRoutedLocalPlayer(ControllerId);
    UCommonUIActionRouter* Router = Player ? Player->GetSubsystem<UCommonUIActionRouter>() : nullptr;
    if (Router && !Router->NativeShouldBlockActionInput()) Reply = FReply::Handled();
}
