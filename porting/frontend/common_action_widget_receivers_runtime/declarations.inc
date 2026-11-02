public:
    void NativeSetBindingKey(int32 NewKey);
    void NativeRefreshInputAction();
    void NativeBindInputMethodChanged(bool bEnable);
protected:
    UCommonInputSubsystem* NativeResolveInputSubsystem() const;
    void NativeHandleInputMethodChanged(ECommonInputType InputType);
public:
    int32 NativeBindingKey;
    bool NativeHidden;
