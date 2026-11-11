// CONFIDENCE: HIGH - validate-then-dispatch is exact; the implementation (ShotLogFlags reset, index to INDEX_NONE) is fully read (dep AFortWeapon_ServerResetShotReport_Implementation).
// Original addresses: exec 0x142b48200; validate vtable +0x638 -> 0x1406801d0; implementation vtable +0x640 -> 0x1426b0cc0.
if (!ServerResetShotReport_Validate())
{
    RPC_ValidateFailed(TEXT("ServerResetShotReport_Validate"));
    return;
}
ServerResetShotReport_Implementation();
