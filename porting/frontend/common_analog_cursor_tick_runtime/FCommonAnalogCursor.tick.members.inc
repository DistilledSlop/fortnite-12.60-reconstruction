// Include Rendering/SlateRenderTransform.h; forward declare UWidget.
// Replace the prior FInputEvent override with these actual original overrides.
protected:
    virtual bool IsRelevantInput(const FAnalogInputEvent& Event) const override; // virtual +68
    virtual bool IsRelevantInput(const FKeyEvent& Event) const override; // virtual +70
private:
    void NativeUpdateCursorVisibility(); // 1407ec060
    bool NativeShouldHandleInput() const; // 1407cf210
    void NativeSetCursorPositionInViewport(const FVector2D& NormalizedPosition); // 1407f8f80
    virtual EOrientation NativeGetScrollOrientation(const UWidget* Widget) const; // 1407b8c50
    TWeakPtr<SWidget> NativeLastCursorTarget; // original +a8/+b0
    FSlateRenderTransform NativeLastCursorTargetTransform; // +b8..cf, identity ctor
    float NativeTimeUntilScrollUpdate = 0.0f; // +d0
