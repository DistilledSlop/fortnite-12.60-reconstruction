// CONFIDENCE: HIGH. Original 0x1426933f0; names proven by static FName initializers.
if (NewState == 1)
{
    if (OldState == 0) OnStartCharge();
    OnChargeUp();
}
else if (NewState == 4)
{
    OnChargeDown();
}
else if (NewState == 2)
{
    AFortPawn* Pawn = Cast<AFortPawn>(GetInstigator());
    if (!Pawn || (!Pawn->IsLocallyControlled() && GetLocalRole() != ROLE_Authority))
    {
        OnRemoteClientReachedMaxCharge();
    }
}
else if (NewState == 3)
{
    OnReachedMaxChargeForCurAmmo();
}
if (OldState != 0 && NewState == 0) OnEndCharge();
