// Original address: 0x141c0c310 (ABuildingProp_LockDevice::HandleMinigameStateChanged)
// Offset evidence: [RCX+0xb99] is an unreflected bool gap in the own region (NativeRecoveredBool_0xb99); state byte is R8B.
// CONFIDENCE: HIGH
const uint8 State = static_cast<uint8>(NewMinigameState);
if ((State == 0 || State == 5) && NativeRecoveredBool_0xb99)
{
    NativeRecoveredBool_0xb99 = false;
}
