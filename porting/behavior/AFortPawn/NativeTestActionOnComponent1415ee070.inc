// CONFIDENCE: HIGH. Complete shared action-gate body.
// Original 0x1415ee070, shared by the pawn and character vehicle entry points.
// The static container is intentionally shared across both call sites and reentry.
static FGameplayTagContainer ActionTags;
ActionTags.Reset();
ActionTags.AddTag(ActionTag);
return !Component->AreAbilityTagsBlocked(ActionTags);
