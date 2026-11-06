// CONFIDENCE: HIGH
// Native 0x1422defe0: forward directly to the component implementation.
if (UFortAbilitySystemComponent* ASC = AbilitySystemComponent)
{
    ASC->NetMulticast_InvokeGameplayCuesExecuted_WithParams_Implementation(GameplayCueTags, PredictionKey, GameplayCueParameters);
}
