// Both original Panel default vtable entries +4E8/+4F0 target 1406801c0 (RET 0).
void UCommonActivatablePanel::NativeAddedToActivationStack() {}
void UCommonActivatablePanel::NativeRemovedFromActivationStack() {}
void UCommonActivatablePanel::NativeResetPanelOperations()
{
    bIsActive = false;
    bNativeIsIntroed = false;
    NativeClearHeldAction();
    NativePreActivation.Unbind();
    OnWidgetActivated.Clear();
    BP_OnWidgetActivated.Clear();
    NativePostActivation.Unbind();
    NativePreDeactivation.Unbind();
    OnWidgetDeactivated.Clear();
    BP_OnWidgetDeactivated.Clear();
    NativePostDeactivation.Unbind();
}
