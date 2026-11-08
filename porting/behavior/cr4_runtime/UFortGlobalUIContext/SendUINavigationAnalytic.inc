// Behaviour snippet: UFortGlobalUIContext::SendUINavigationAnalytic (body only; spliced into the generated member function).
// Original address: 0x144ce6510 (thunk 0x14524b0c0)
// Evidence: calls NativeGetContextPlayerController144cb7c20(false) (0x144cb7c20). If the result is non-null the body calls 0x1406801c0 (bare RET, no effect) with (controller, Destination, bUserInitiated); that call has no effect, so only the lookup call remains and its result is discarded.
// CONFIDENCE: HIGH
(void)NativeGetContextPlayerController144cb7c20(false);
(void)Destination;
(void)bUserInitiated;
