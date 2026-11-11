// CONFIDENCE: MEDIUM Null target returns false. The target is cast to the gameplay tag asset interface (class cast FUN_142ec9900 with the interface class), then the interface tag query at vtable slot 0x28 is called with the container.
// Original: exec 0x1428f8080 (decompile captures/0da226f5ed8442fb9ce91e4985f6c96a/0x1428f8080.c)
// Slot 0x28 is inferred as HasAnyMatchingGameplayTags from the interface vtable order; the exact method was not confirmed against the runtime name table.
if (Target == nullptr)
{
	return false;
}
const IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(Target);
return TagInterface != nullptr && TagInterface->HasAnyMatchingGameplayTags(Tags);
