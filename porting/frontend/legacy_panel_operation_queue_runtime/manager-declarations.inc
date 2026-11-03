public:
    // Source names for original native multicasts +30/+48/+60/+90.
    FCommonLegacyPanelChanged NativePanelRemoved;
    FSimpleMulticastDelegate NativePanelStackChanged;
    FCommonLegacyPanelChanged NativePanelActivated;
    FCommonLegacyInputSuspensionChanged NativeInputSuspensionChanged;
    void NativeInternalPushActivatablePanel(UCommonActivatablePanel* Panel, bool bIntroPanel,
        bool bActivatePanel, bool bOutroPanelBelow);
    void NativeBeginOperationProcessing();
    void NativeStartNextOperation();
    void NativeResetPanelOperations();
    // true: original +FD branch handled this invocation, including its resume tick.
    // false: caller must dispatch the actual held-handler interface tick (+28).
    bool NativeTickOperationSuspension();
private:
    TArray<UCommonActivatablePanel*> NativePredictedPanelStack() const;
    void NativeAddCleanOperation(const FOperation& Op);
    void NativeBindPanelActivation(UCommonActivatablePanel* Panel, const FGuid& Id);
    void NativePrePanelActivated(UCommonActivatablePanel* Panel, FGuid Id);
    void NativePostPanelActivated(UCommonActivatablePanel* Panel, FGuid Id);
    void NativePrePanelDeactivatedForPop(UCommonActivatablePanel* Panel, FGuid Id);
    void NativePostPanelDeactivatedForPop(UCommonActivatablePanel* Panel, FGuid Id);
    void NativePostPanelDeactivatedForPush(UCommonActivatablePanel* Panel, FGuid Id);
    bool bNativeSuspendStartingOperations = false; // +E0
    bool bNativeProcessingOperations = false; // +FC
    int32 NativeOutstandingPanelPops = 0; // +F8
    uint32 NativeIdleOperationTicks = 0; // +100
