// Behaviour snippet: AFortPlayerStartupController::HandlePlayerStartupControllerReset_Implementation (body only).
// Original address: 0x141f374f0 (thunk 0x142737e10)
// Offset evidence: this+0x280 = AFortPlayerStartupController::OnResetContainmentTimer (reflected FPlayerStartupControllerDestroyed, no params; asm passes RDX=0 to the broadcast 0x1406a9dc0).
// Verbosity-gated diagnostic log at 0x142c54b70 elided (no state change)
// CONFIDENCE: MEDIUM
OnResetContainmentTimer.Broadcast();
