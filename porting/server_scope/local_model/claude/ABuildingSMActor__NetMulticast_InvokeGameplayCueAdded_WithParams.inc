// CONFIDENCE: HIGH
// Original: exec thunk 0x142756cd0 dispatches to ABuildingSMActor virtual +0xa18 (real body 0x141c106a0).
// That body forwards to the component: UFortAbilitySystemComponent virtual +0x548 (0x1406d0920).
// Same forward as AFortPawn::NetMulticast_InvokeGameplayCueAdded_WithParams_Implementation (ported). Null component: no-op.
if (UFortAbilitySystemComponent* ASC = Cast<UFortAbilitySystemComponent>(AbilitySystemComponent))
{
    ASC->NetMulticast_InvokeGameplayCueAdded_WithParams_Implementation(GameplayCueTag, PredictionKey, Parameters);
}
