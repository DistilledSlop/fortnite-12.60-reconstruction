// CONFIDENCE: HIGH
// Native 0x1422ded60: forward directly to the component implementation.
if (UFortAbilitySystemComponent* ASC = AbilitySystemComponent)
{
    ASC->NetMulticast_InvokeGameplayCueExecuted_WithParams_Implementation(GameplayCueTag, PredictionKey, GameplayCueParameters);
}
