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
