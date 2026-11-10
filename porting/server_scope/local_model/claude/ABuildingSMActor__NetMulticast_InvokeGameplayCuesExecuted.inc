// CONFIDENCE: HIGH
// Original: exec thunk 0x1427576c0 dispatches to ABuildingSMActor virtual +0xa38 (real body 0x141c109d0).
// That body forwards to the component: UFortAbilitySystemComponent virtual +0x568 (0x1406d0cf0).
// Same forward as AFortPawn::NetMulticast_InvokeGameplayCuesExecuted_Implementation (ported). Null component: no-op.
if (UFortAbilitySystemComponent* ASC = Cast<UFortAbilitySystemComponent>(AbilitySystemComponent))
{
    ASC->NetMulticast_InvokeGameplayCuesExecuted_Implementation(GameplayCueTags, PredictionKey, EffectContext);
}
