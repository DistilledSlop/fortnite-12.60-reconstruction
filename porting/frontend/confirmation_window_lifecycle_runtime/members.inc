public:
    FNativeConfirmationWindowClosed NativeConfirmationWindowClosed;
    FFortDialogExternalLatentActionHandle NativeDialogLatentHandle = [] {
        FFortDialogExternalLatentActionHandle Handle;
        Handle.Handle = INDEX_NONE;
        return Handle;
    }();
    void NativeClearConfirmation144f664f0();
    void NativeDismissConfirmation(EFortDialogResult Result, FName ActionName = NAME_None);
    void NativeResetConfirmation144f75f90();
    void NativeRefreshConfirmationDisplay144f7b690();
    void NativeRemoveConfirmationButtons144f555e0();
    void NativeCloseConfirmation144f55ae0();
protected:
    virtual void NativeHandleActivated() override;
private:
    UFUNCTION(BlueprintCallable)
    void BotClickButton();
    // Unresolved lower boundaries, intentionally no fallback body.
    bool NativeBotControllerEligible144f54030() const;
    void NativeConfirmationButtonClicked144f5fbc0(UCommonButton* Button);
    void NativeConfirmLatent144f5fbc0(UCommonButton* Button, FName ActionName);
    void NativeDeclineButtonClicked144f5fce0(UCommonButton* Button);
