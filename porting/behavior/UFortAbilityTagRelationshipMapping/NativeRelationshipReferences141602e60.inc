// CONFIDENCE: HIGH. Original141602e60. HasAny direction includes relationship parent tags.
for (const FFortAbilityTagRelationship& Relationship : AbilityTagRelationships)
{
    if (Relationship.AbilityTag == TargetTag &&
        (Relationship.AbilityTagsToBlock.HasAny(AbilityTags) ||
         Relationship.AbilityTagsToCancel.HasAny(AbilityTags)))
    {
        return true;
    }
}
return false;
