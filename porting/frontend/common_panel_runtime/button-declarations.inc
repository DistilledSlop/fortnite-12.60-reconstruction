public:
    UCommonButtonInternal(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
    // Project-local names for original private native delegate fields.
    FOnClicked NativeDoubleClickDelegate;      // original Win64 +428
    FSimpleDelegate NativeFocusReceivedDelegate; // original Win64 +448
    void NativeSetInteractionEnabled(bool bEnabled);
protected:
    virtual void ReleaseSlateResources(bool bReleaseChildren) override;
    virtual TSharedRef<SWidget> RebuildWidget() override;
    bool IsCommonInputSuspended();
    FReply SlateHandleCommonClicked();
    FReply SlateHandleCommonDoubleClicked();
    void SlateHandleCommonPressed();
    void SlateHandleCommonReleased();
    void SlateHandleCommonFocusReceived();
    // Independently owning fields; inherited UButton::MyButton owns the same child.
    TSharedPtr<SBox> NativeButtonBox;             // original Win64 +468
    TSharedPtr<SCommonButton> NativeCommonButton; // original Win64 +478
