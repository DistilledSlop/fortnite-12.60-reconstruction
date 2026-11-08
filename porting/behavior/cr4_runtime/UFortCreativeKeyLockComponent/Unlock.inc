// Behaviour snippet: UFortCreativeKeyLockComponent::Unlock (body only; spliced into the generated member function).
// Original address: 0x141d95510 (thunk 0x1428c5c20)
// Evidence: NativeHelper1440b2120(this) (0x1440b2120) must be >= 3 and Pawn must be non-null; then returns CurrentState (byte +0x138, ECreativeKeyLockState) == 1. Every other path returns false.
// Verbosity-gated diagnostic log at 0x142c54b70 (guarded by CMP byte [0x1481286e0],3) elided (no state change).
// CONFIDENCE: MEDIUM
static_assert(sizeof(UFortMinigameObjectiveComponent) > 0, "Generated header include for UFortMinigameObjectiveComponent.");
if (UFortMinigameObjectiveComponent::NativeHelper1440b2120(this) >= 3 && Pawn != nullptr)
{
    return static_cast<uint8>(CurrentState) == 1;
}
return false;
