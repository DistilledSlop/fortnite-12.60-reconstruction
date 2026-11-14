// CONFIDENCE: HIGH. Complete original body; class lifecycle acceptance remains separate.
UFortAbilitySystemComponent* Component = AbilitySystemComponent;
if (!Component) return false;
// Original static container1480fa6e0 is shared across calls, including reentry.
static FGameplayTagContainer ActionTags;
ActionTags.Reset();
ActionTags.AddTag(ActionTag);
return !Component->AreAbilityTagsBlocked(ActionTags);
