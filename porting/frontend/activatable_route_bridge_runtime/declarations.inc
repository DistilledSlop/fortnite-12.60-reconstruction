public:
    // Native, non-reflected route hooks. Names describe their recovered roles.
    virtual TOptional<TOptional<uint8>> NativeGetActionRouteOverride() const;
    virtual TOptional<FCommonActionRouteInputConfig> NativeGetActionRouteInputConfig() const;
    virtual UWidget* NativeGetActionRouteDesiredFocusTarget() const;
    virtual void ReleaseSlateResources(bool bReleaseChildren) override;
    FSimpleMulticastDelegate NativeWidgetReleased;
protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeConstruct() override;
    virtual bool NativeHandleBackAction();
private:
    void NativeDispatchBackAction();
