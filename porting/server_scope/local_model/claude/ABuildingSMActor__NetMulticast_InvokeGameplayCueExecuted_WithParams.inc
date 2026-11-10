// CONFIDENCE: HIGH
// Original: exec thunk 0x142757210 dispatches to ABuildingSMActor virtual +0xa30 (real body 0x141c10860).
// That body forwards to the component: UFortAbilitySystemComponent virtual +0x560 (0x1406d0b00).
// Same forward as AFortPawn::NetMulticast_InvokeGameplayCueExecuted_WithParams_Implementation (ported). Null component: no-op.
if (UFortAbilitySystemComponent* ASC = Cast<UFortAbilitySystemComponent>(AbilitySystemComponent))
{
    ASC->NetMulticast_InvokeGameplayCueExecuted_WithParams_Implementation(GameplayCueTag, PredictionKey, GameplayCueParameters);
}
