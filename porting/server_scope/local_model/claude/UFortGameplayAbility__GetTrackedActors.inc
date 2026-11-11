// CONFIDENCE: MEDIUM. Original exec 0x1428f8990; core 0x141600340.
// Returns the still-valid actors tracked under this ability TrackingId; empty when there is no ASC or no group.
TArray<AActor*> TrackedActors;
UFortAbilitySystemComponent* AbilitySystem = GetTrackingAbilitySystemComponent();
if (IsValid(AbilitySystem) && TrackingId != INDEX_NONE)
{
	if (const TArray<FFortTrackedActorEntry>* Entries = AbilitySystem->FindTrackedActorEntriesById(TrackingId))
	{
		TrackedActors = AbilitySystem->CollectValidTrackedActors(*Entries);
	}
}
return TrackedActors;
