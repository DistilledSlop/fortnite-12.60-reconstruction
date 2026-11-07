// Behaviour snippet: AFortPlayerStartupController::HandleAllInCountdownInitiated_Implementation (body only).
// Original address: 0x141f36e40 (thunk 0x142711840)
// Offset evidence: this+0x260 = AFortPlayerStartupController::OnAllInCountdownInitiated (reflected FAllInCountdownInitiated, no params; asm passes RDX=0 to the broadcast 0x1406a9dc0).
// Verbosity-gated diagnostic log at 0x142c54b70 elided (no state change)
// CONFIDENCE: MEDIUM
OnAllInCountdownInitiated.Broadcast();
