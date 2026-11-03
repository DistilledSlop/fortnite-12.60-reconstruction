public:
    UFUNCTION(BlueprintCallable, meta=(WorldContext="InWorldContextObject", BlueprintInternalUseOnly="true"))
    static UFortAsyncAction_ShowConfirmation_NUI* ShowSimpleConfirmationDialog_CustomInput(
        UObject* InWorldContextObject, FSlateBrush Icon, FText Title, FText Message,
        FName ConfirmAction, FName DeclineAction, UWidget* AdditionalContent,
        UWidget* LeftAdditionalContent, bool Cancelable, ULocalPlayer* TargetPlayer);
    virtual void Activate() override;
private:
    bool bNativeRestoreExternalInputSuspension = false;
    void NativeOnConfirmationResult(EFortDialogResult Result, FName ResultName);
