// 14235f5a0. Unlike InputKey, original touch invokes its manager delegate
// before delegating to UGameViewportClient, without controller-id remapping.
bool UFortGameViewportClient::InputTouch(FViewport* Viewport, int32 ControllerId,
    uint32 Handle, ETouchType::Type Type, const FVector2D& TouchLocation,
    float Force, FDateTime DeviceTimestamp, uint32 TouchpadIndex)
{
    if (NativeRouteTouchDelegate.IsBound())
    {
        FReply Reply = FReply::Unhandled();
        NativeRouteTouchDelegate.Execute(ControllerId, Handle, Type, TouchLocation, Reply);
        if (Reply.IsEventHandled()) return true;
    }
    return Super::InputTouch(Viewport, ControllerId, Handle, Type, TouchLocation,
        Force, DeviceTimestamp, TouchpadIndex);
}
