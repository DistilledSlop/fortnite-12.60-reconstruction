public:
    UCommonLazyImage();
    virtual void SetBrush(const FSlateBrush& InBrush) override;
    virtual void SetBrushFromAsset(USlateBrushAsset* Asset) override;
    virtual void SetBrushFromTexture(UTexture2D* Texture, bool bMatchSize = false) override;
    virtual void SetBrushFromTextureDynamic(UTexture2DDynamic* Texture, bool bMatchSize = false) override;
    virtual void SetBrushFromMaterial(UMaterialInterface* Material) override;
    virtual void SynchronizeProperties() override;
    virtual void ReleaseSlateResources(bool bReleaseChildren) override;
protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void OnWidgetRebuilt() override;
    virtual TSharedRef<SWidget> NativeRebuildImage();
    virtual void CancelImageStreaming() override;
    virtual void OnImageStreamingStarted(TSoftObjectPtr<UObject> SoftObject) override;
    virtual void OnImageStreamingComplete(TSoftObjectPtr<UObject> SoftObject) override;
private:
    void NativeLoadingStateChanged(bool bLoading);
    void NativeSetTexture(class UTexture* Texture, bool bMatchSize);
    void NativeRestoreDefaultImage();
    TSharedPtr<class SLoadGuard> NativeLoadGuard;
