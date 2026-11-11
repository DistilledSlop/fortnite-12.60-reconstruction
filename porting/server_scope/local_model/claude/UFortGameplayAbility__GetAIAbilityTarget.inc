// CONFIDENCE: LOW Returns the cached AI ability target when one is valid (entry at ability +0x908, count at +0x910). Otherwise resolves the owning pawn, its AI component chain and the named AI target sub-object, and returns it when it is the expected class. Null otherwise.
// Original: exec 0x1428f8210 (decompile captures/0da226f5ed8442fb9ce91e4985f6c96a/0x1428f8210.c)
// The fallback chain was read at the structure level; the component names were not resolved.
if (CachedAIAbilityTargets.Num() > 0 && CachedAIAbilityTargets[0].IsValid())
{
	return CachedAIAbilityTargets[0].Get();
}
APawn* OwnerPawn = Cast<APawn>(CurrentActorInfo != nullptr ? CurrentActorInfo->OwnerActor.Get() : nullptr);
if (OwnerPawn == nullptr)
{
	return nullptr;
}
return GetAIAbilityTargetFallback(OwnerPawn);
