// CONFIDENCE: MEDIUM
// Original address: 0x142331eb0; virtual slot +0xcd8.
if (UFortAbilitySystemComponent* Component = AbilitySystemComponent)
{
    return Component->HasNativeAction(ActionTag);
}
return false;
