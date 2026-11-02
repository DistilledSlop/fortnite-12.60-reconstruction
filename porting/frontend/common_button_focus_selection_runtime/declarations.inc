public:
    FSimpleMulticastDelegate NativeHovered;
    FSimpleMulticastDelegate NativeUnhovered;
    FSimpleMulticastDelegate NativeFocusReceived;
protected:
    UFUNCTION()
    void HandleButtonClicked();
    UFUNCTION()
    void HandleButtonPressed();
    UFUNCTION()
    void HandleButtonReleased();
    virtual void NativeHandleFocusReceived();
    virtual void NativeOnHovered();
    virtual void NativeOnUnhovered();
    virtual void NativeUpdateActionWidget();
    virtual void NativeDispatchTriggeredAction();
    void NativeRefreshActionWidgetVisibility();
    void NativeSetSelectedInternal(bool bSelected, bool bGiveClickFeedback, bool bBroadcast);
    void NativeSetIsSelected(bool bSelected, bool bGiveClickFeedback);
    FDataTableRowHandle NativeTriggeredInputAction;
