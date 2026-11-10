// CONFIDENCE: HIGH
// Original: exec thunk 0x1427567a0 dispatches to ABuildingSMActor virtual +0xa20 (real body 0x141c105f0).
// That body forwards to the component: UFortAbilitySystemComponent virtual +0x550 (0x1406d0860).
// Same forward as AFortPawn::NetMulticast_InvokeGameplayCueAdded_Implementation (ported). Null component: no-op.
if (UFortAbilitySystemComponent* ASC = Cast<UFortAbilitySystemComponent>(AbilitySystemComponent))
{
    ASC->NetMulticast_InvokeGameplayCueAdded_Implementation(GameplayCueTag, PredictionKey, EffectContext);
}
