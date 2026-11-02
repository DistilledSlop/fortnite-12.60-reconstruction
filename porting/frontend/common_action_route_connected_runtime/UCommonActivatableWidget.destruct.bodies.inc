void UCommonActivatableWidget::NativeDestruct()
{
    if (IsActivated()) NativeDeactivateRequest();
    if (NativeGetRegisteredActionBindingHandles().Num() > 0)
        if (auto* Player = GetOwningLocalPlayer())
            if (auto* Router = Player->GetSubsystem<UCommonUIActionRouter>()) Router->NativeRemoveWidgetActionBindings(this);
    // Original1407d1c55 tail-calls UUserWidget directly, bypassing CommonUserWidget.
    UUserWidget::NativeDestruct();
}
