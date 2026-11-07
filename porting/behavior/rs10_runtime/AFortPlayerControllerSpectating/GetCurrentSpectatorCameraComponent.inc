// Behaviour snippet: AFortPlayerControllerSpectating::GetCurrentSpectatorCameraComponent (body only; spliced into the generated member function).
// Original body address: 0x1423d3c20 (thunk 0x142a6b960, group B, flags 0x54020401).
// Offset evidence: this+0x540 = SpectatorPawn (APlayerController, Object, reflected). The pawn's +0x2a0 = SpectatorCameraComponent (AFortReplaySpectatorPawnBase, reflected). The asm checks IsA(AFortReplaySpectatorPawnBase::StaticClass()) (call 0x142acab80) before reading +0x2a0; otherwise returns null.
// CONFIDENCE: HIGH
if (SpectatorPawn != nullptr && SpectatorPawn->IsA(AFortReplaySpectatorPawnBase::StaticClass()))
{
    return static_cast<AFortReplaySpectatorPawnBase*>(SpectatorPawn)->SpectatorCameraComponent;
}
return nullptr;
