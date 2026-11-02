public:
    UCommonActionWidget(const FObjectInitializer& ObjectInitializer);
    virtual void ReleaseSlateResources(bool bReleaseChildren) override;

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void OnWidgetRebuilt() override;

    TSharedPtr<SBox> NativeBox;
    TSharedPtr<SImage> NativeIcon;
    TSharedPtr<SImage> NativeProgress;
    TSharedPtr<SImage> NativeRim;
    FSlateBrush NativeCachedBrush;
