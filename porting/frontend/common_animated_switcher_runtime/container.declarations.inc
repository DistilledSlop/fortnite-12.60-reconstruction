public:
void NativeCreateAnimatedSwitcher1407e7d70();
bool NativeHasSwitcher1c8() const;
int32 NativeSwitcherIndex1407c19a0() const;
void NativeSwitcherTransition140800440(int32 Index, bool bInstantTransition);
void NativeSwitcherIndexChanged1407c61f0(int32 Index);
void NativeSwitcherTransitioningChanged1407cbc30(bool bTransitioning);
void NativeSetActiveIndex1407f9d70(int32 Index, bool bInstantTransition);
void NativeRemoveSlateWidget1407eefa0(TSharedRef<SWidget> Widget);
bool NativeReleaseDeferredSlate1407bc5a0(float DeltaTime);
void NativeDisplayedWidgetDeactivated1407c65f0(UCommonActivatableWidget* Widget);
virtual void ReleaseSlateResources(bool bReleaseChildren) override;
protected:
virtual TSharedRef<SWidget> RebuildWidget() override;
public:
