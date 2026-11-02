public:
    int32 NativeRegisterActionBinding(const FCommonActionBindingDescriptor& Descriptor); // 1407ed3f0
    const TArray<int32>& NativeGetRegisteredActionBindingHandles() const { return NativeRegisteredActionBindingHandles; }
protected:
    virtual void OnWidgetRebuilt() override; // original +2b0 = 1407e03f0
    virtual void NativeDestruct() override; // original +320 = 1407d1e90
