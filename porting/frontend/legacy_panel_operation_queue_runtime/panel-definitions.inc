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

// Original +4C0 (0x1407e1130): validity check of the weak manager at +3B0, then the manager's
// PopActivatablePanel body (0x1407e0c80, the same body behind the reflected exec 0x140810760).
void UCommonActivatablePanel::NativePopSelfFromInputManager1407e1130()
{
    if (NativeInputManager.IsValid())
    {
        NativeInputManager.Get()->PopActivatablePanel(this);
    }
}
