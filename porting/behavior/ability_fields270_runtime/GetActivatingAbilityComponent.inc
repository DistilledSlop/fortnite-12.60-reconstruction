// CONFIDENCE: high
// 0x1415f7d70: the actor info and its weak component may both be null.
return CurrentActorInfo ? Cast<UFortAbilitySystemComponent>(CurrentActorInfo->AbilitySystemComponent.Get()) : nullptr;
