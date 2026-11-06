// CONFIDENCE: HIGH
// Native 0x1422dedf0: forward directly to the component implementation.
if (UFortAbilitySystemComponent* ASC = AbilitySystemComponent)
{
    ASC->NetMulticast_InvokeGameplayCuesAddedAndWhileActive_WithParams_Implementation(GameplayCueTags, PredictionKey, GameplayCueParameters);
}
