// Behaviour snippet: UBuildingStructuralSupportSystem::IsBuildingActorValid (static body only).
// Original body address: 0x141c82140 (thunk 0x14275cac0). Null or pending-kill fails.
// Offset evidence: [Actor+0x3e1] signed byte compared with 0 by JL; negative (bit 7 set) fails. Unreflected gap in ABuildingSMActor (member_field NativeRecoveredUint8_0x3e1).
// CONFIDENCE: MEDIUM
static_assert(sizeof(ABuildingSMActor) > 0, "ABuildingSMActor must be complete for the member read");
if (!ActorToTest || ActorToTest->IsPendingKill())
{
	return false;
}
return (ActorToTest->NativeRecoveredUint8_0x3e1 & 0x80) == 0;
