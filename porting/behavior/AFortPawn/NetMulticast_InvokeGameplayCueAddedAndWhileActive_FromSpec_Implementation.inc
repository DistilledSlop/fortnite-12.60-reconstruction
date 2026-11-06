// CONFIDENCE: HIGH
// Native 0x1422dea20: forward directly to the component implementation.
if (UFortAbilitySystemComponent* ASC = AbilitySystemComponent)
{
    ASC->NetMulticast_InvokeGameplayCueAddedAndWhileActive_FromSpec_Implementation(Spec, PredictionKey);
}
