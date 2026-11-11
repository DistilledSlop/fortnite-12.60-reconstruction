// CONFIDENCE: MEDIUM. Original exec 0x1428f8a10; core 0x141600470.
// Returns the still-valid actors tracked under GroupTag; empty for an invalid tag, missing ASC or missing group.
TArray<AActor*> TrackedActors;
UFortAbilitySystemComponent* AbilitySystem = GetTrackingAbilitySystemComponent();
if (IsValid(AbilitySystem) && GroupTag.IsValid())
{
	if (const TArray<FFortTrackedActorEntry>* Entries = AbilitySystem->FindTrackedActorEntriesByGroup(GroupTag))
	{
		TrackedActors = AbilitySystem->CollectValidTrackedActors(*Entries);
	}
}
return TrackedActors;
