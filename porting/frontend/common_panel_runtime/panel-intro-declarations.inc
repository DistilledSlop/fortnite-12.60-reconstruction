protected:
    virtual void OnBeginIntro_Implementation();
    virtual void NativeActivateRequest() override;
    virtual void NativeHandleActivated() override;
    virtual void NativeIntroComplete();
    virtual void NativeInputMethodChangedHook(ECommonInputType InputType);
    virtual uint32 NativeGetGlobalInputPriorityFilter() const;
private:
    friend class UCommonInputManager;
#if !UE_BUILD_SHIPPING && !UE_BUILD_TEST
    friend struct FPanelIntroProbeAccess;
#endif
    void NativeInputMethodChanged(ECommonInputType InputType);
    bool bNativeIsIntroed = false;
    TWeakObjectPtr<UCommonInputManager> NativeInputManager;
    FDelegateHandle NativeInputMethodHandle;
    FCommonPanelOperationCallback NativePreActivation;
    FCommonPanelOperationCallback NativePostActivation;
