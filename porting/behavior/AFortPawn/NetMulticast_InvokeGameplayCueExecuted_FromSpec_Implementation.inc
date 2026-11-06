// CONFIDENCE: HIGH
// Native 0x1422dec30: forward directly to the component implementation.
if (UFortAbilitySystemComponent* ASC = AbilitySystemComponent)
{
    ASC->NetMulticast_InvokeGameplayCueExecuted_FromSpec_Implementation(Spec, PredictionKey);
}
