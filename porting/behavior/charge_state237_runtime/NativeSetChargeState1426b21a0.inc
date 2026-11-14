// CONFIDENCE: HIGH. Original 0x1426b21a0, including callback/RPC/copy/update ordering.
const uint8 OldState = static_cast<uint8>(NativeLocalChargeStatusPack906);
if (OldState != NewState)
{
    const uint8 Sequence = static_cast<uint8>((NativeLocalChargeStatusPack906 >> 8) + 1u);
    NativeLocalChargeStatusPack906 = static_cast<uint16>((static_cast<uint16>(Sequence) << 8) | NewState);
    NativeNotifyChargeState1426933f0(OldState, NewState);
    if (AFortPawn* Pawn = Cast<AFortPawn>(GetInstigator()))
    {
        if (Pawn->IsLocallyControlled() && GetLocalRole() != ROLE_Authority)
        {
            ServerSetChargeState(NewState);
        }
    }
    // The event/RPC may reenter; original copies the CURRENT local pack here.
    FMemory::Memcpy(&ChargeStatusPack, &NativeLocalChargeStatusPack906, sizeof(ChargeStatusPack));
    ForceNetUpdate();
}
