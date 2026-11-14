// CONFIDENCE: HIGH. Original1426a5d60; current spec handle is read after IsActive.
UFortGameplayAbility* Ability = ActiveAbility;
return Ability && Ability->IsActive()
    && ActiveAbility->NativeCurrentSpecHandle() == ReloadAbilitySpecHandle;
