// CONFIDENCE: MEDIUM
// Original address: 0x1415f7bf0.
// Append matching block and cancel tags, retaining the caller's existing tags.
for (const FFortAbilityTagRelationship& Relationship : AbilityTagRelationships)
{
    if (AbilityTags.HasTag(Relationship.AbilityTag))
    {
        if (BlockTags)
        {
            BlockTags->AppendTags(Relationship.AbilityTagsToBlock);
        }
        if (CancelTags)
        {
            CancelTags->AppendTags(Relationship.AbilityTagsToCancel);
        }
    }
}
