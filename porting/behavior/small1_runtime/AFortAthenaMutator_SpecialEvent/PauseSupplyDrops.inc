// Original address: 0x141b283a0 (AFortAthenaMutator_SpecialEvent::PauseSupplyDrops)
// Offset evidence: [RCX+0xf0] is Role (reflected); [RCX+0x364] is an unreflected bool gap (NativeRecoveredBool_0x364).
// CONFIDENCE: HIGH
if (static_cast<uint8>(Role) != 3)
{
    return;
}
NativeRecoveredBool_0x364 = bPause;
