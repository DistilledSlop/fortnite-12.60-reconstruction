void UCommonButton::NativeOnDoubleClicked()
{
    BP_OnDoubleClicked();
    NativeDoubleClicked.Broadcast();
    if (OnButtonDoubleClicked.IsBound())
    {
        OnButtonDoubleClicked.Broadcast(this);
    }
}

FReply UCommonButton::NativeHandleDoubleClicked()
{
    bStopDoubleClickPropagation = false;
    NativeOnDoubleClicked();
    return bStopDoubleClickPropagation ? FReply::Handled() : FReply::Unhandled();
}
