// UCommonActivatableWidgetContainerBase: original 0x1407f9d70, specialized
// by the root's 0x1407ae7f0 to index 0 and bInstantTransition=false.
void NativeSetActiveIndex1407f9d70(int32 Index, bool bInstantTransition);
// Original container byte +0x1e8; initialized by its native constructor.
bool NativePendingDeactivation1e8 = false;
// Real Slate switcher accessors must be implemented by the recovered switcher;
// these do not have fallback bodies.
bool NativeHasSwitcher1c8() const;
int32 NativeSwitcherIndex1407c19a0() const;
void NativeSwitcherTransition140800440(int32 Index, bool bInstantTransition);
