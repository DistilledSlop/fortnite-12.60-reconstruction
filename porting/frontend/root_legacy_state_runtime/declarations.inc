// UFortRootViewportLayout, unreflected bytes before NativeDormant2aa.
uint8 NativeLegacyState2a8 = 0;
uint8 NativePendingLegacyState2a9 = 0;
void NativeSetLegacyState14516f380(uint8 Selector);
virtual void NativeChooseLegacyState480(uint8& Selector, UClass*& StateClass);
void NativeAttachState145174c00(UUserWidget* StateWidget);
FString NativeLegacyStateName14514e200(uint8 State) const;
// Requires the original log-category identity; intentionally no fallback body.
void NativeLogLegacyStateChange14516f4d1(uint8 PreviousState, uint8 NewState) const;

// UFortUIStateWidget_NUI. These are real virtual contracts, not empty defaults.
virtual void NativeActivateLegacyState510(uint8 PreviousState);
virtual void NativeDeactivateLegacyState518(uint8 NextState);
