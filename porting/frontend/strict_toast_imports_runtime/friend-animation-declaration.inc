private:
    // Native close-state byte at +0x288, set before OnPlayCloseAnimation.
    bool bNativeToastClosing = false;

    UFUNCTION()
    void HandleOpenCloseAnimationFinished();

    UFortUINotification* NativeToastNotification = nullptr;

    void NativeCloseSubscriptionToast();
