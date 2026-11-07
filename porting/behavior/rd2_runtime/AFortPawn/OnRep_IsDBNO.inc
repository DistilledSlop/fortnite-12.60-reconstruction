// Behaviour snippet: AFortPawn::OnRep_IsDBNO (body only; spliced into the generated member function).
// Original address: 0x14233c370 (thunk 0x142a1eba0). Asm: porting/generated/native-cap-virtual3/0x14233c370.asm
// Offset evidence: bIsDBNO = bitfield at +0x53a (mask 0x2, reflected); bPlayedDying = bitfield at +0x520 (mask 0x10, reflected; the asm clears it with AND 0xef).
// Offset evidence: OnPlayerStartDBNO = FOnPlayerStartDBNO (no params) at +0x6a8, called only when bIsDBNO is set.
// Offset evidence: OnDBNOStateChanged = FOnDBNOStateChangedEvent (AFortPawn*, bool) at +0x6b8, always called; params packed at [RSP+0x20] as (this, bIsDBNO).
// CONFIDENCE: HIGH
if (bIsDBNO)
{
    OnPlayerStartDBNO.Broadcast();
}
else
{
    bPlayedDying = false;
}
OnDBNOStateChanged.Broadcast(this, bIsDBNO);
