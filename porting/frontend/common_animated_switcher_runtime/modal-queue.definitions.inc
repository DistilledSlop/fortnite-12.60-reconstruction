// UCommonActivatableWidgetContainerBase; exact 0x1407f9d70 control flow.
// Requires the real Slate switcher accessors declared in modal-queue.declarations.inc.
void UCommonActivatableWidgetContainerBase::NativeSetActiveIndex1407f9d70(
    int32 Index, bool bInstantTransition)
{
    if (!NativeHasSwitcher1c8() || NativeSwitcherIndex1407c19a0() == Index)
        return;
    if (DisplayedWidget)
    {
        DisplayedWidget->NativeWidgetDeactivated.RemoveAll(this);
        if (DisplayedWidget->bIsActive)
            DisplayedWidget->DeactivateWidget();
        else if (NativeSwitcherIndex1407c19a0() != 0)
            NativePendingDeactivation1e8 = true;
    }
    NativeSwitcherTransition140800440(Index, bInstantTransition);
}

void UCommonActivatableWidgetContainerBase::ClearWidgets()
{
    NativeSetActiveIndex1407f9d70(0, false);
}

void NativeClearModalQueue1407ae7f0(UCommonActivatableWidgetQueue* Queue)
{
    Queue->ClearWidgets();
}
