// CONFIDENCE: HIGH. Complete original body; build/runtime pending.
// Original 0x14232c970. Even an empty query returns false if ASC is absent.
UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
return ASC && ASC->HasAllMatchingGameplayTags(TagContainer);
