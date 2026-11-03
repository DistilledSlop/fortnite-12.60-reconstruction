public:
    UFUNCTION(BlueprintCallable)
    void SetTriggeringInputAction(FDataTableRowHandle InputActionRow);
protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeRegisterInputMethodListener(); // original+470
    virtual void NativeUnregisterInputMethodListener(); // original+478
    virtual void NativeHandleButtonInputMethodChanged(ECommonInputType InputType); // original+480
    virtual void NativeBindTriggeringInputAction(); // original+488
    UFUNCTION()
    virtual void HandleTriggeringActionCommited(bool& bPassThrough); // original+490
    UFUNCTION()
    virtual void NativeOnActionProgress(float HeldPercent); // original+4f8
    UFUNCTION()
    virtual void NativeOnActionComplete(); // original+500
    UFUNCTION(BlueprintImplementableEvent)
    void OnActionProgress(float HeldPercent);
    UFUNCTION(BlueprintImplementableEvent)
    void OnActionComplete();
private:
    int32 NativeTriggeringActionBinding = INDEX_NONE; // original+acc
    void NativeUnbindTriggeringInputAction(); // 140801750
