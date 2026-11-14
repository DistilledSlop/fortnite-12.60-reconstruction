// CONFIDENCE: HIGH. Original14234f020; local-control query follows targeting callbacks.
if (GetLocalRole() != ROLE_SimulatedProxy)
{
    NativeApplyTargeting1423304f0(bNewTargeting);
    if (IsLocallyControlled())
    {
        NativeTargetingChanged15f0.Broadcast(bNewTargeting);
    }
}
