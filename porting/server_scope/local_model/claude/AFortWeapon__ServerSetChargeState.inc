// CONFIDENCE: HIGH - validate-then-dispatch is exact; the charge-state transitions are MEDIUM (begin on 0->1, end on 4) with LOW side effects in BeginWeaponCharge/EndWeaponCharge.
// Original addresses: exec 0x142b48250; validate vtable +0x650 -> 0x1406801d0; implementation vtable +0x658 -> 0x1426b0d00.
if (!ServerSetChargeState_Validate(NewState))
{
    RPC_ValidateFailed(TEXT("ServerSetChargeState_Validate"));
    return;
}
ServerSetChargeState_Implementation(NewState);
