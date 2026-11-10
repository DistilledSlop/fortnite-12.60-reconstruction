// CONFIDENCE: high
// Native body 0x1423483e0 (reflected wrapper 0x142a1f430); mirror of AddNonReplicatedTagToPawn (0x142302420).
// Original: ASC = interface subobject (this+0x4c8) vtable[0x10] GetAbilitySystemComponent; if non-null, ASC->GameplayTagCountContainer (ASC+0x1040)
// UpdateTagCount(Tag, -1, false) and, only when it reports a change, ASC vtable[0x810] OnTagUpdated(Tag, false).
// That sequence is UAbilitySystemComponent::UpdateTagMap(Tag, -1), which RemoveLooseGameplayTag(Tag) (Count = 1) calls.
if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
{
    ASC->RemoveLooseGameplayTag(Tag);
}
