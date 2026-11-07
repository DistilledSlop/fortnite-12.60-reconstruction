// Behaviour snippet: UHUDLayoutToolContext::CloseLayoutTool (body only; spliced into the generated member function).
// Original address: 0x144d2f7d0 (thunk 0x14532adb0)
// Offset evidence: OnCloseLayoutTool (FOnCloseLayoutTool) is the reflected delegate at +0x48 (UHUDLayoutToolContext.h). The call at 0x1406a9dc0 broadcasts it with no parameters (RDX = 0). The Reason FString is read only by the elided log; its buffer free at 0x142bee710 is the compiler-emitted destructor.
// Verbosity-gated diagnostic log at 0x142c54b70 elided (no state change).
// CONFIDENCE: MEDIUM
OnCloseLayoutTool.Broadcast();
