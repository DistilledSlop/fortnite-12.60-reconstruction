// Behaviour snippet: UFortMinigameObjectiveComponent::SetIsInteractionAllowed (body only; spliced into the generated member function).
// Original address: 0x1420a3e30 (thunk 0x1429d3460)
// Evidence: if NativeHelper1440b2120(this) (0x1440b2120) == 3: the argument (DL) is stored at byte +0x1a2, which is the declared bitfield bIsInteractionAllowed (mask 1); the declared bit member is written. Then OnObjectiveAllowedInteractionChanged (+0x188, FMinigameObjectiveUpdated) is broadcast with this (0x1406a9dc0). Otherwise nothing happens.
// CONFIDENCE: HIGH
if (UFortMinigameObjectiveComponent::NativeHelper1440b2120(this) == 3)
{
    bIsInteractionAllowed = bInIsInteractionAllowed;
    OnObjectiveAllowedInteractionChanged.Broadcast(this);
}
