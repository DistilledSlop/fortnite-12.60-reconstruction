// CONFIDENCE: HIGH. Complete recovered predicate.
// Original 0x1426a5c50. Preserve the second ActiveAbility read after IsActive.
UFortGameplayAbility* Ability = ActiveAbility;
return Ability && Ability->IsActive()
    && ActiveAbility->NativeCurrentSpecHandle() != ReloadAbilitySpecHandle;
