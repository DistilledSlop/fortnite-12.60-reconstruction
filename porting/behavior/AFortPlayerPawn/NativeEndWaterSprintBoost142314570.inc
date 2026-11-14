// CONFIDENCE: HIGH. Original142314570; only active-bit clearing is role gated.
if (bIsWaterSprintBoost)
{
    if (GetLocalRole() != ROLE_SimulatedProxy)
    {
        bIsWaterSprintBoost = false;
    }
    if (NiagaraPlayerWaterBoost && NiagaraPlayerWaterBoost->IsActive())
    {
        NiagaraPlayerWaterBoost->Deactivate();
    }
    OnEndWaterSprintBoost();
    ForceNetUpdate();
}
bIsWaterSprintBoostPending = false;
NativeWaterSprintBoostTimeRemaining12d4 = 0.0f;
