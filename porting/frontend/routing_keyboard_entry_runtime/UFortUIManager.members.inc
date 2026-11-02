// Native methods corresponding to original manager callbacks. These fragments
// require the real viewport delegate declarations and route router methods.
private:
    ULocalPlayer* NativeFindRoutedLocalPlayer(int32 ControllerId) const;
    void NativeRouteTouch(int32 ControllerId, uint32 Handle, ETouchType::Type Type,
        const FVector2D& TouchLocation, FReply& Reply);
    void NativeRouteAxis(int32 ControllerId, FKey Key, float Delta, FReply& Reply);
