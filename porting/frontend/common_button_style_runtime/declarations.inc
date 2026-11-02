protected:
    UCommonButtonStyle* NativeGetStyleCDO() const;
    void NativeRebuildStyle();
    void NativeRefreshCurrentStyle();
    void NativeRefreshDimensions();
    // Original native virtual slot 0x4f0 forwards to the reflected event.
    virtual void NativeOnCurrentTextStyleChanged();
    friend struct FCommonButtonStyleRebuildProbeAccess;
