public:
    UCommonActivatableWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
    // Project-local names: original private native delegate names are unavailable.
    FCommonWidgetActivationNative NativeWidgetActivated;
    FCommonWidgetActivationNative NativeWidgetDeactivated;
protected:
    virtual void NativeActivateRequest();
    virtual void NativeDeactivateRequest();
    virtual void NativeHandleActivated();
    virtual void NativeHandleDeactivated();
