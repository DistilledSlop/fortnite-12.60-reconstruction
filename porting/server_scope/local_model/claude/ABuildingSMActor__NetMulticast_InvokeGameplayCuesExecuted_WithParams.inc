// CONFIDENCE: HIGH
// Original: exec thunk 0x142757890 dispatches to ABuildingSMActor virtual +0xa28 (real body 0x141c10ae0).
// That body forwards to the component: UFortAbilitySystemComponent virtual +0x558 (0x1406d0e00).
// Same forward as AFortPawn::NetMulticast_InvokeGameplayCuesExecuted_WithParams_Implementation (ported). Null component: no-op.
if (UFortAbilitySystemComponent* ASC = Cast<UFortAbilitySystemComponent>(AbilitySystemComponent))
{
    ASC->NetMulticast_InvokeGameplayCuesExecuted_WithParams_Implementation(GameplayCueTags, PredictionKey, GameplayCueParameters);
}
