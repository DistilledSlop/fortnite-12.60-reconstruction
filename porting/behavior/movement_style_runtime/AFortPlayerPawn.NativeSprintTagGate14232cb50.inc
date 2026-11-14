// CONFIDENCE: high. Original 0x14232cb50.
// Original +0x4c8 interface +0x10 is GetAbilitySystemComponent; retain its dispatch.
UFortAbilitySystemComponent* Component = Cast<UFortAbilitySystemComponent>(GetAbilitySystemComponent());
return Component && Component->HasMatchingGameplayTag(
    FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x690));
