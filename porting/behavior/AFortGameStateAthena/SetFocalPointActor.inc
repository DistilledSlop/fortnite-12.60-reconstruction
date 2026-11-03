// Behaviour snippet: AFortGameStateAthena::SetFocalPointActor (body only).
// Original address: 0x142920d00; native body 0x1419ae6f0.
// Decompile source: porting/generated/decomp/AFortGameStateAthena/SetFocalPointActor.c
// Evidence: porting/behavior/game_state_queries_evidence.md
// CONFIDENCE: HIGH
if (GetLocalRole() == ROLE_Authority)
{
	FocalPointOffset = InFocalPointOffset;
	RemainingFocalPointActorDuration = InDuration;
	FocalPointFOV = InFocalPointFOV;
	FocalPointActor = InFocalPointActor;
}
