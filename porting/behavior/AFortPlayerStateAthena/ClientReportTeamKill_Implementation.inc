// Original address: 0x141a3d8f0; evidence: porting/behavior/pawn_state_evidence.md.
// CONFIDENCE: MEDIUM
AFortPlayerControllerAthena* Controller = Cast<AFortPlayerControllerAthena>(GetOwner());
if (Controller != nullptr)
{
	Controller->TeamKillsReportedNative.Broadcast(TeamKills);
}
