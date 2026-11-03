// CONFIDENCE: HIGH
// Native 0x1422deba0: forward directly to the component implementation.
if (UFortAbilitySystemComponent* ASC = AbilitySystemComponent)
{
    ASC->NetMulticast_InvokeGameplayCueAdded_WithParams_Implementation(GameplayCueTag, PredictionKey, Parameters);
}
