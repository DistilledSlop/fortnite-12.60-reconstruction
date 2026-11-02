void UCommonButton::NativeOnSelected(bool bBroadcast)
{
    BP_OnSelected();
    if (bBroadcast)
    {
        NativeSelectedChanged.Broadcast(true);
        if (OnSelectedChanged.IsBound()) OnSelectedChanged.Broadcast(this, true);
    }
    NativeOnCurrentTextStyleChanged();
}

void UCommonButton::NativeOnDeselected(bool bBroadcast)
{
    BP_OnDeselected();
    if (bBroadcast)
    {
        NativeSelectedChanged.Broadcast(false);
        if (OnSelectedChanged.IsBound()) OnSelectedChanged.Broadcast(this, false);
    }
    NativeOnCurrentTextStyleChanged();
}
