void UFortFriendSubscriptionToastWidget::HandleOpenCloseAnimationFinished()
{
    if (bNativeToastClosing)
    {
        bNativeToastClosing = false;
        OnToastFinished.Broadcast();
    }
}

void UFortFriendSubscriptionToastWidget::NativeCloseSubscriptionToast()
{
    if (AFortPlayerControllerAthena* Controller = Cast<AFortPlayerControllerAthena>(GetOwningPlayer()))
    {
        if (Controller->SubscriptionNudgeInputComponent)
        {
            Controller->PopInputComponent(Controller->SubscriptionNudgeInputComponent);
        }
    }
    bNativeToastClosing = true;
    NativeToastNotification = nullptr;
    OnPlayCloseAnimation();
}
