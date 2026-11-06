// CONFIDENCE: HIGH
// Native 0x1422deaf0: forward directly to the component implementation.
if (UFortAbilitySystemComponent* ASC = AbilitySystemComponent)
{
    ASC->NetMulticast_InvokeGameplayCueAdded_Implementation(GameplayCueTag, PredictionKey, EffectContext);
}
