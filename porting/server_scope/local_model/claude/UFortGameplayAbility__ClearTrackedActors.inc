// CONFIDENCE: MEDIUM Authority-only. Looks up this ability tracking entry (tracking id stored on the ability) in the owning actor tracked-actor groups, destroys the grouped actors when bDestroyActors is set, then removes the id.
// Original: exec 0x1428f7e90 (decompile captures/0da226f5ed8442fb9ce91e4985f6c96a/0x1428f7e90.c)
if (!HasAuthority(&CurrentActivationInfo))
{
	return;
}
ClearTrackedActorGroupById(TrackingId, bDestroyActors);
