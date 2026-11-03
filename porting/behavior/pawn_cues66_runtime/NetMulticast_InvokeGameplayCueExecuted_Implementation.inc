// CONFIDENCE: HIGH
// Native 0x1422decb0: forward directly to the component implementation.
if (UFortAbilitySystemComponent* ASC = AbilitySystemComponent)
{
    ASC->NetMulticast_InvokeGameplayCueExecuted_Implementation(GameplayCueTag, PredictionKey, EffectContext);
}
