// Original item owner/lifetime behavior; see HANDOFF.md.
// CONFIDENCE: HIGH
AFortPlayerController* Controller = nullptr;
if (NativeOwningController.IsValid())
{
    Controller = static_cast<AFortPlayerController*>(NativeOwningController.Get());
}
return Controller;
