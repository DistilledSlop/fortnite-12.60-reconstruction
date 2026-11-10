// CONFIDENCE: HIGH
// Original: exec thunk 0x142757440 dispatches to ABuildingSMActor virtual +0xa00 (real body 0x141c108f0).
// That body forwards to the component: UFortAbilitySystemComponent virtual +0x530 (0x1406d0b70).
// Same forward as AFortPawn::NetMulticast_InvokeGameplayCuesAddedAndWhileActive_WithParams_Implementation (ported). Null component: no-op.
if (UFortAbilitySystemComponent* ASC = Cast<UFortAbilitySystemComponent>(AbilitySystemComponent))
{
    ASC->NetMulticast_InvokeGameplayCuesAddedAndWhileActive_WithParams_Implementation(GameplayCueTags, PredictionKey, GameplayCueParameters);
}
