// Behaviour snippet: UBuildingStructuralSupportSystem::IsBuildingActorValid (static body only).
// Original body address: 0x141c82140 (thunk 0x14275cac0). Null or pending-kill fails.
// Reflected ABuildingActor::bDestroyed is byte +0x3e1 mask 0x80. Native JL rejects bit 7 set.
// CONFIDENCE: MEDIUM
static_assert(sizeof(ABuildingSMActor) > 0, "ABuildingSMActor must be complete for the member read");
if (!ActorToTest || ActorToTest->IsPendingKill())
{
	return false;
}
return !ActorToTest->bDestroyed;
