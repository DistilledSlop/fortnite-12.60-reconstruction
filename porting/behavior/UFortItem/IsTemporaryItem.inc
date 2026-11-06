// CONFIDENCE: HIGH
// Recovered item stack/ownership behavior; see HANDOFF.md.
AFortPlayerController* Controller = GetOwningController();
return !Controller || bTemporaryItemOwningController;
