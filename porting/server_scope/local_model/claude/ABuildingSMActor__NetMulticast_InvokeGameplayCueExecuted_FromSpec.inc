// CONFIDENCE: MEDIUM
// Original: exec thunk 0x142757080 dispatches to ABuildingSMActor virtual +0xa48 (real body 0x141c10730).
// The forwarder first copies Spec (FUN_1406ed110, copy constructor) and destroys the copy after the call (FUN_1406ae6c0, destructor). The copy is made in source by value.
// That body forwards to the component: UFortAbilitySystemComponent virtual +0x578 (0x1406d09d0).
// Same forward as AFortPawn::NetMulticast_InvokeGameplayCueExecuted_FromSpec_Implementation (ported). Null component: no-op.
if (UFortAbilitySystemComponent* ASC = Cast<UFortAbilitySystemComponent>(AbilitySystemComponent))
{
    ASC->NetMulticast_InvokeGameplayCueExecuted_FromSpec_Implementation(Spec, PredictionKey);
}
