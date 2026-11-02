public:
    virtual bool IsHovered() const override;
protected:
    // Original constructor clears selected and initializes the weak internal button.
    bool bNativeSelected = false;
    TWeakObjectPtr<UCommonButtonInternal> NativeInternalButton;
    friend struct FCommonButtonTextStyleProbeAccess;
