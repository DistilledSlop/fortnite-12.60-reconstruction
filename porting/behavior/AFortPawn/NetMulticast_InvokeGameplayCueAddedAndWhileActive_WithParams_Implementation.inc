// CONFIDENCE: HIGH
// Native 0x1422dea60: forward directly to the component implementation.
if (UFortAbilitySystemComponent* ASC = AbilitySystemComponent)
{
    ASC->NetMulticast_InvokeGameplayCueAddedAndWhileActive_WithParams_Implementation(GameplayCueTag, PredictionKey, GameplayCueParameters);
}
