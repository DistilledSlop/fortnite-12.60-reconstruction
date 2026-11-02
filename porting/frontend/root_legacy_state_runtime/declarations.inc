// UFortRootViewportLayout, unreflected bytes before NativeDormant2aa.
uint8 NativeLegacyState2a8;
uint8 NativePendingLegacyState2a9;
void NativeSetLegacyState14516f380(uint8 Selector);
virtual void NativeChooseLegacyState480(uint8& Selector, UClass*& StateClass);
void NativeAttachState145174c00(UUserWidget* StateWidget);
FString NativeLegacyStateName14514e200(uint8 State) const;

// UFortUIStateWidget_NUI. These are real virtual contracts, not empty defaults.
virtual void NativeActivateLegacyState510(uint8 PreviousState);
virtual void NativeDeactivateLegacyState518();
