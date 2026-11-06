// Original item owner/lifetime behavior; see HANDOFF.md.
// CONFIDENCE: HIGH
(void)NativeOwningController.IsValid();
bTemporaryItemOwningController = true;
AFortPlayerController* Controller = InController;
NativeOwningController = Controller;
