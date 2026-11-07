// Original address: 0x1418f1270 (AFortAthenaGoatVehicle::IsBoosting, receiver delta 944)
// Offset evidence: object offset 0x10b4 bool gap (NativeRecoveredBool_0x10b4) from [RCX+0xd04]; object offset 0x10a0 float gap (NativeRecoveredFloat_0x10a0) from [RCX+0xcf0].
// Constant 0.0001f per decompile FUN_1418f1270.
// CONFIDENCE: HIGH
if (!NativeRecoveredBool_0x10b4 && NativeRecoveredFloat_0x10a0 > 0.0001f)
{
    return true;
}
return false;
