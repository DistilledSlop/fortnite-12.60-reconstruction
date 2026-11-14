// CONFIDENCE: HIGH. Complete original body; build/runtime pending.
// Original 0x14232c9f0. The missing ASC case returns false.
UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
return ASC && ASC->HasMatchingGameplayTag(TagToCheck);
