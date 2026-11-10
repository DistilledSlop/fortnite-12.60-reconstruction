// CONFIDENCE: MEDIUM
// Original: exec thunk 0x142756920 dispatches to ABuildingSMActor virtual +0xa10 (real body 0x141c10520).
// The forwarder body passes the ASC an 8-byte value taken from its second argument; the exec builds Spec with FUN_1406ed400 (default ctor). Source-level argument order follows the AFortPawn analog.
// That body forwards to the component: UFortAbilitySystemComponent virtual +0x540 (0x1406d0780).
// Same forward as AFortPawn::NetMulticast_InvokeGameplayCueAddedAndWhileActive_FromSpec_Implementation (ported). Null component: no-op.
if (UFortAbilitySystemComponent* ASC = Cast<UFortAbilitySystemComponent>(AbilitySystemComponent))
{
    ASC->NetMulticast_InvokeGameplayCueAddedAndWhileActive_FromSpec_Implementation(Spec, PredictionKey);
}
