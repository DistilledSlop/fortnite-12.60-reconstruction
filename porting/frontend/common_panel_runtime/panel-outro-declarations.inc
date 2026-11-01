protected:
    virtual void OnBeginOutro_Implementation();
    virtual void NativeDeactivateRequest() override;
    virtual void NativeHandleDeactivated() override;
    virtual void NativeOutroComplete();
private:
#if !UE_BUILD_SHIPPING && !UE_BUILD_TEST
    friend struct FPanelOutroProbeAccess;
#endif
    void NativeClearHeldAction();
    float NativeHeldActionDuration = 0.0f;
    FDataTableRowHandle NativeHeldAction;
    FCommonPanelActionMap NativeActionRecords;
    FCommonPanelOperationCallback NativePreDeactivation;
    FCommonPanelOperationCallback NativePostDeactivation;
