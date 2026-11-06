// CONFIDENCE: HIGH
// Native 0x1422deed0: forward directly to the component implementation.
if (UFortAbilitySystemComponent* ASC = AbilitySystemComponent)
{
    ASC->NetMulticast_InvokeGameplayCuesExecuted_Implementation(GameplayCueTags, PredictionKey, EffectContext);
}
