// Original constructor 140796950: bounded reflected/default initialization.
// Unreflected selection/delegate/lifecycle storage is not reconstructed here.
UCommonButton::UCommonButton(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bApplyAlphaOnDisable = true;
    bSelectable = false;
    bShouldSelectUponReceivingFocus = false;
    bToggleable = false;
    bDisplayInputActionWhenNotInteractable = true;
    bShouldUseFallbackDefaultInputAction = true;
    bIsFocusable = true;
}

bool UCommonButton::IsInteractionEnabled() const
{
    const ESlateVisibility Visibility = GetVisibility();
    return GetIsEnabled() && bNativeButtonEnabled && bNativeInteractionEnabled
        && Visibility != ESlateVisibility::Collapsed && Visibility != ESlateVisibility::Hidden;
}

bool UCommonButton::NativeIsInteractionEnabled() const
{
    return IsInteractionEnabled();
}
