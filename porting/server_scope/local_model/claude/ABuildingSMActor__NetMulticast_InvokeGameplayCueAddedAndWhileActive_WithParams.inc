// CONFIDENCE: HIGH
// Original: exec thunk 0x142756aa0 dispatches to ABuildingSMActor virtual +0xa08 (real body 0x141c10560).
// That body forwards to the component: UFortAbilitySystemComponent virtual +0x538 (0x1406d07e0).
// Same forward as AFortPawn::NetMulticast_InvokeGameplayCueAddedAndWhileActive_WithParams_Implementation (ported). Null component: no-op.
if (UFortAbilitySystemComponent* ASC = Cast<UFortAbilitySystemComponent>(AbilitySystemComponent))
{
    ASC->NetMulticast_InvokeGameplayCueAddedAndWhileActive_WithParams_Implementation(GameplayCueTag, PredictionKey, GameplayCueParameters);
}
