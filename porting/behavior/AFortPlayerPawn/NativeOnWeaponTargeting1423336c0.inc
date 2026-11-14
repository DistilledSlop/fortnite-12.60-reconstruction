// CONFIDENCE: HIGH. Original1423336c0; role and local-control queries are intentionally repeated.
if (GetLocalRole() != ROLE_SimulatedProxy)
{
    if (IsLocallyControlled() && GetLocalRole() != ROLE_SimulatedProxy)
    {
        NativeApplyTargeting1423304f0(bNewTargeting);
        if (IsLocallyControlled())
        {
            NativeTargetingChanged15f0.Broadcast(bNewTargeting);
        }
    }
}
