void UCommonButton::ClearSelection()
{
    NativeSetSelectedInternal(false, false, true);
}

void UCommonButton::SetIsSelected(bool InSelected, bool bGiveClickFeedback)
{
    NativeSetIsSelected(InSelected, bGiveClickFeedback);
}

void UCommonButton::SetSelectedInternal(bool bInSelected, bool bAllowSound, bool bBroadcast)
{
    NativeSetSelectedInternal(bInSelected, bAllowSound, bBroadcast);
}
