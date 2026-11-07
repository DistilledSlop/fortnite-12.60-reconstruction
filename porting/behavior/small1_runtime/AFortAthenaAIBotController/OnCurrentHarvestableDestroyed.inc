// Original address: 0x1416a2580 (AFortAthenaAIBotController::OnCurrentHarvestableDestroyed)
// Offset evidence: DamagedActor (RDX) vs [RCX+0x820] CurrentLootActor (reflected Object); InstigatedBy (R9) vs this (RCX); [RCX+0x818] is an unreflected uint8 gap (NativeRecoveredUInt8_0x818).
// CONFIDENCE: HIGH
if (DamagedActor == CurrentLootActor && InstigatedBy == this)
{
    NativeRecoveredUInt8_0x818 = 2;
    return;
}
NativeRecoveredUInt8_0x818 = 3;
